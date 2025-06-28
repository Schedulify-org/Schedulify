#include "db_schedules.h"

DatabaseScheduleManager::DatabaseScheduleManager(QSqlDatabase& database) : db(database) {
}

bool DatabaseScheduleManager::insertSchedule(const InformativeSchedule& schedule, const string& scheduleName,
                                             const vector<int>& sourceFileIds) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule insertion");
        return false;
    }

    // Create a schedule set if we don't have one
    string setName = scheduleName.empty() ? "Generated Schedules " +
                                            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString()
                                          : scheduleName;

    int setId = createScheduleSet(setName, sourceFileIds);
    if (setId <= 0) {
        Logger::get().logError("Failed to create schedule set");
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO schedule 
        (schedule_set_id, schedule_index, schedule_name, schedule_data_json, 
         amount_days, amount_gaps, gaps_time, avg_start, avg_end, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
    )");

    string scheduleJson = DatabaseJsonHelpers::scheduleToJson(schedule);

    query.addBindValue(setId);
    query.addBindValue(schedule.index);
    query.addBindValue(QString::fromStdString(scheduleName));
    query.addBindValue(QString::fromStdString(scheduleJson));
    query.addBindValue(schedule.amount_days);
    query.addBindValue(schedule.amount_gaps);
    query.addBindValue(schedule.gaps_time);
    query.addBindValue(schedule.avg_start);
    query.addBindValue(schedule.avg_end);

    if (!query.exec()) {
        Logger::get().logError("Failed to insert schedule: " + query.lastError().text().toStdString());
        return false;
    }

    // Update schedule count in set
    updateScheduleSetCount(setId);

    Logger::get().logInfo("Schedule inserted successfully with set ID: " + std::to_string(setId));
    return true;
}

bool DatabaseScheduleManager::insertSchedules(const vector<InformativeSchedule>& schedules, const string& setName,
                                              const vector<int>& sourceFileIds) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule insertion");
        return false;
    }

    if (schedules.empty()) {
        Logger::get().logWarning("No schedules to insert");
        return true;
    }

    // Create schedule set
    string actualSetName = setName.empty() ?
                           "Generated Schedules " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString()
                                           : setName;

    int setId = createScheduleSet(actualSetName, sourceFileIds);
    if (setId <= 0) {
        Logger::get().logError("Failed to create schedule set");
        return false;
    }

    // Start transaction
    if (!db.transaction()) {
        Logger::get().logError("Failed to begin transaction for schedule insertion");
        return false;
    }

    int successCount = 0;
    for (const auto& schedule : schedules) {
        QSqlQuery query(db);
        query.prepare(R"(
            INSERT INTO schedule 
            (schedule_set_id, schedule_index, schedule_data_json, 
             amount_days, amount_gaps, gaps_time, avg_start, avg_end, created_at, updated_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
        )");

        string scheduleJson = DatabaseJsonHelpers::scheduleToJson(schedule);

        query.addBindValue(setId);
        query.addBindValue(schedule.index);
        query.addBindValue(QString::fromStdString(scheduleJson));
        query.addBindValue(schedule.amount_days);
        query.addBindValue(schedule.amount_gaps);
        query.addBindValue(schedule.gaps_time);
        query.addBindValue(schedule.avg_start);
        query.addBindValue(schedule.avg_end);

        if (query.exec()) {
            successCount++;
        } else {
            Logger::get().logError("Failed to insert schedule index " + std::to_string(schedule.index) +
                                   ": " + query.lastError().text().toStdString());
        }
    }

    if (successCount == 0) {
        Logger::get().logError("Failed to insert any schedules, rolling back transaction");
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        Logger::get().logError("Failed to commit schedule insertion transaction");
        db.rollback();
        return false;
    }

    // Update schedule count in set
    updateScheduleSetCount(setId);

    Logger::get().logInfo("Successfully inserted " + std::to_string(successCount) + "/" +
                          std::to_string(schedules.size()) + " schedules with set ID: " + std::to_string(setId));

    return successCount == static_cast<int>(schedules.size());
}

bool DatabaseScheduleManager::updateSchedule(int scheduleId, const InformativeSchedule& schedule) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule update");
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        UPDATE schedule 
        SET schedule_data_json = ?, amount_days = ?, amount_gaps = ?, gaps_time = ?, 
            avg_start = ?, avg_end = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )");

    string scheduleJson = DatabaseJsonHelpers::scheduleToJson(schedule);

    query.addBindValue(QString::fromStdString(scheduleJson));
    query.addBindValue(schedule.amount_days);
    query.addBindValue(schedule.amount_gaps);
    query.addBindValue(schedule.gaps_time);
    query.addBindValue(schedule.avg_start);
    query.addBindValue(schedule.avg_end);
    query.addBindValue(scheduleId);

    if (!query.exec()) {
        Logger::get().logError("Failed to update schedule: " + query.lastError().text().toStdString());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    if (rowsAffected == 0) {
        Logger::get().logWarning("No schedule found with ID: " + std::to_string(scheduleId));
        return false;
    }

    Logger::get().logInfo("Schedule updated successfully (ID: " + std::to_string(scheduleId) + ")");
    return true;
}

bool DatabaseScheduleManager::deleteSchedule(int scheduleId) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule deletion");
        return false;
    }

    // Get the set ID before deletion to update count
    int setId = -1;
    QSqlQuery getSetQuery(db);
    getSetQuery.prepare("SELECT schedule_set_id FROM schedule WHERE id = ?");
    getSetQuery.addBindValue(scheduleId);
    if (getSetQuery.exec() && getSetQuery.next()) {
        setId = getSetQuery.value(0).toInt();
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM schedule WHERE id = ?");
    query.addBindValue(scheduleId);

    if (!query.exec()) {
        Logger::get().logError("Failed to delete schedule: " + query.lastError().text().toStdString());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    if (rowsAffected == 0) {
        Logger::get().logWarning("No schedule found with ID: " + std::to_string(scheduleId));
        return false;
    }

    // Update schedule count in set
    if (setId > 0) {
        updateScheduleSetCount(setId);
    }

    Logger::get().logInfo("Schedule deleted successfully (ID: " + std::to_string(scheduleId) + ")");
    return true;
}

bool DatabaseScheduleManager::deleteAllSchedules() {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule deletion");
        return false;
    }

    QSqlQuery query("DELETE FROM schedule", db);
    if (!query.exec()) {
        Logger::get().logError("Failed to delete all schedules: " + query.lastError().text().toStdString());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    Logger::get().logInfo("Deleted all schedules from database (" + std::to_string(rowsAffected) + " schedules)");
    return true;
}

bool DatabaseScheduleManager::deleteSchedulesBySetId(int setId) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule deletion");
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM schedule WHERE schedule_set_id = ?");
    query.addBindValue(setId);

    if (!query.exec()) {
        Logger::get().logError("Failed to delete schedules for set ID " + std::to_string(setId) +
                               ": " + query.lastError().text().toStdString());
        return false;
    }

    int deletedCount = query.numRowsAffected();

    // Update schedule count in set
    updateScheduleSetCount(setId);

    Logger::get().logInfo("Deleted " + std::to_string(deletedCount) + " schedules for set ID: " + std::to_string(setId));
    return true;
}

int DatabaseScheduleManager::createScheduleSet(const string& setName, const vector<int>& sourceFileIds) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule set creation");
        return -1;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO schedule_set (set_name, source_file_ids_json, created_at, updated_at)
        VALUES (?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
    )");

    // Convert file IDs to JSON
    QJsonArray jsonArray;
    for (int id : sourceFileIds) {
        jsonArray.append(id);
    }
    QJsonDocument doc(jsonArray);
    string fileIdsJson = doc.toJson(QJsonDocument::Compact).toStdString();

    query.addBindValue(QString::fromStdString(setName));
    query.addBindValue(QString::fromStdString(fileIdsJson));

    if (!query.exec()) {
        Logger::get().logError("Failed to create schedule set: " + query.lastError().text().toStdString());
        return -1;
    }

    QVariant lastId = query.lastInsertId();
    if (!lastId.isValid()) {
        Logger::get().logError("Failed to get schedule set ID after creation");
        return -1;
    }

    int setId = lastId.toInt();
    Logger::get().logInfo("Schedule set created: '" + setName + "' with ID: " + std::to_string(setId));
    return setId;
}

vector<InformativeSchedule> DatabaseScheduleManager::getSchedulesBySetId(int setId) {
    vector<InformativeSchedule> schedules;
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule retrieval");
        return schedules;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, schedule_index, schedule_data_json, amount_days, amount_gaps, 
               gaps_time, avg_start, avg_end
        FROM schedule 
        WHERE schedule_set_id = ? 
        ORDER BY schedule_index
    )");
    query.addBindValue(setId);

    if (!query.exec()) {
        Logger::get().logError("Failed to retrieve schedules: " + query.lastError().text().toStdString());
        return schedules;
    }

    while (query.next()) {
        schedules.push_back(createScheduleFromQuery(query));
    }

    Logger::get().logInfo("Retrieved " + std::to_string(schedules.size()) +
                          " schedules for set ID: " + std::to_string(setId));
    return schedules;
}

vector<ScheduleSetEntity> DatabaseScheduleManager::getAllScheduleSets() {
    vector<ScheduleSetEntity> sets;
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule set retrieval");
        return sets;
    }

    QSqlQuery query(R"(
        SELECT id, set_name, source_file_ids_json, schedule_count, created_at, updated_at
        FROM schedule_set 
        ORDER BY created_at DESC
    )", db);

    while (query.next()) {
        sets.push_back(createScheduleSetFromQuery(query));
    }

    Logger::get().logInfo("Retrieved " + std::to_string(sets.size()) + " schedule sets from database");
    return sets;
}

bool DatabaseScheduleManager::deleteScheduleSet(int setId) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule set deletion");
        return false;
    }

    // Get set details before deletion for logging
    ScheduleSetEntity set = getScheduleSetById(setId);
    if (set.id == 0) {
        Logger::get().logError("Schedule set with ID " + std::to_string(setId) + " not found");
        return false;
    }

    // Due to CASCADE DELETE, deleting the set will automatically delete all schedules
    QSqlQuery query(db);
    query.prepare("DELETE FROM schedule_set WHERE id = ?");
    query.addBindValue(setId);

    if (!query.exec()) {
        Logger::get().logError("Failed to delete schedule set: " + query.lastError().text().toStdString());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    if (rowsAffected == 0) {
        Logger::get().logWarning("No schedule set found with ID: " + std::to_string(setId));
        return false;
    }

    Logger::get().logInfo("Schedule set deleted successfully: '" + set.set_name + "' (ID: " + std::to_string(setId) + ")");
    return true;
}

InformativeSchedule DatabaseScheduleManager::createScheduleFromQuery(QSqlQuery& query) {
    int scheduleIndex = query.value(1).toInt();
    string scheduleJson = query.value(2).toString().toStdString();
    int amountDays = query.value(3).toInt();
    int amountGaps = query.value(4).toInt();
    int gapsTime = query.value(5).toInt();
    int avgStart = query.value(6).toInt();
    int avgEnd = query.value(7).toInt();

    return DatabaseJsonHelpers::scheduleFromJson(scheduleJson, 0, scheduleIndex,
                                                 amountDays, amountGaps, gapsTime,
                                                 avgStart, avgEnd);
}

ScheduleSetEntity DatabaseScheduleManager::createScheduleSetFromQuery(QSqlQuery& query) {
    ScheduleSetEntity set;
    set.id = query.value(0).toInt();
    set.set_name = query.value(1).toString().toStdString();
    set.source_file_ids_json = query.value(2).toString().toStdString();
    set.schedule_count = query.value(3).toInt();
    set.created_at = query.value(4).toDateTime();
    set.updated_at = query.value(5).toDateTime();
    return set;
}

bool DatabaseScheduleManager::updateScheduleSetCount(int setId) {
    QSqlQuery countQuery(db);
    countQuery.prepare("SELECT COUNT(*) FROM schedule WHERE schedule_set_id = ?");
    countQuery.addBindValue(setId);

    if (!countQuery.exec() || !countQuery.next()) {
        Logger::get().logWarning("Failed to count schedules for set ID: " + std::to_string(setId));
        return false;
    }

    int count = countQuery.value(0).toInt();

    QSqlQuery updateQuery(db);
    updateQuery.prepare("UPDATE schedule_set SET schedule_count = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    updateQuery.addBindValue(count);
    updateQuery.addBindValue(setId);

    return updateQuery.exec();
}

bool DatabaseScheduleManager::scheduleExists(int scheduleId) {
    if (!db.isOpen()) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM schedule WHERE id = ?");
    query.addBindValue(scheduleId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

int DatabaseScheduleManager::getScheduleCount() {
    if (!db.isOpen()) {
        return -1;
    }

    QSqlQuery query("SELECT COUNT(*) FROM schedule", db);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

int DatabaseScheduleManager::getScheduleCountBySetId(int setId) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule count by set ID");
        return -1;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM schedule WHERE schedule_set_id = ?");
    query.addBindValue(setId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    Logger::get().logError("Failed to get schedule count for set ID: " + std::to_string(setId));
    return -1;
}

bool DatabaseScheduleManager::scheduleSetExists(int setId) {
    if (!db.isOpen()) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM schedule_set WHERE id = ?");
    query.addBindValue(setId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

int DatabaseScheduleManager::getScheduleSetCount() {
    if (!db.isOpen()) {
        return -1;
    }

    QSqlQuery query("SELECT COUNT(*) FROM schedule_set", db);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

ScheduleSetEntity DatabaseScheduleManager::getScheduleSetById(int setId) {
    ScheduleSetEntity set;

    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule set retrieval");
        return set;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, set_name, source_file_ids_json, schedule_count, created_at, updated_at
        FROM schedule_set
        WHERE id = ?
    )");
    query.addBindValue(setId);

    if (query.exec() && query.next()) {
        set = createScheduleSetFromQuery(query);
    } else {
        Logger::get().logWarning("No schedule set found with ID: " + std::to_string(setId));
    }

    return set;
}

bool DatabaseScheduleManager::updateScheduleSet(int setId, const string& setName) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule set update");
        return false;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE schedule_set SET set_name = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(QString::fromStdString(setName));
    query.addBindValue(setId);

    if (!query.exec()) {
        Logger::get().logError("Failed to update schedule set: " + query.lastError().text().toStdString());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    if (rowsAffected == 0) {
        Logger::get().logWarning("No schedule set found with ID: " + std::to_string(setId));
        return false;
    }

    Logger::get().logInfo("Schedule set updated successfully (ID: " + std::to_string(setId) + ")");
    return true;
}

map<string, int> DatabaseScheduleManager::getScheduleStatistics() {
    map<string, int> stats;

    if (!db.isOpen()) {
        Logger::get().logError("Database not open for statistics");
        return stats;
    }

    // Get basic counts
    stats["total_sets"] = getScheduleSetCount();
    stats["total_schedules"] = getScheduleCount();

    // Get average schedules per set
    QSqlQuery avgQuery("SELECT AVG(schedule_count) FROM schedule_set", db);
    if (avgQuery.exec() && avgQuery.next()) {
        stats["avg_schedules_per_set"] = avgQuery.value(0).toInt();
    }

    // Get schedules created in last 24 hours
    QSqlQuery recentQuery(R"(
        SELECT COUNT(*) FROM schedule
        WHERE created_at >= datetime('now', '-1 day')
    )", db);
    if (recentQuery.exec() && recentQuery.next()) {
        stats["schedules_last_24h"] = recentQuery.value(0).toInt();
    }

    // Get sets created in last 7 days
    QSqlQuery setsQuery(R"(
        SELECT COUNT(*) FROM schedule_set
        WHERE created_at >= datetime('now', '-7 days')
    )", db);
    if (setsQuery.exec() && setsQuery.next()) {
        stats["sets_last_7_days"] = setsQuery.value(0).toInt();
    }

    return stats;
}

vector<InformativeSchedule> DatabaseScheduleManager::getRecentSchedules(int limit) {
    vector<InformativeSchedule> schedules;

    if (!db.isOpen()) {
        Logger::get().logError("Database not open for recent schedules");
        return schedules;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, schedule_index, schedule_data_json, amount_days, amount_gaps,
               gaps_time, avg_start, avg_end
        FROM schedule
        ORDER BY created_at DESC
        LIMIT ?
    )");
    query.addBindValue(limit);

    if (!query.exec()) {
        Logger::get().logError("Failed to retrieve recent schedules: " + query.lastError().text().toStdString());
        return schedules;
    }

    while (query.next()) {
        schedules.push_back(createScheduleFromQuery(query));
    }

    Logger::get().logInfo("Retrieved " + std::to_string(schedules.size()) + " recent schedules");
    return schedules;
}

bool DatabaseScheduleManager::insertSchedulesBulk(const vector<InformativeSchedule>& schedules, int setId) {
    if (!db.isOpen() || schedules.empty()) {
        return false;
    }

    Logger::get().logInfo("Starting bulk insert of " + std::to_string(schedules.size()) + " schedules");

    // Optimize database for bulk operations
    DatabaseUtils::optimizeForBulkInserts(db);

    try {
        // Prepare batch data
        vector<QVariantList> batchData;
        batchData.reserve(schedules.size());

        const QString insertQuery = R"(
            INSERT INTO schedule
            (schedule_set_id, schedule_index, schedule_data_json,
             amount_days, amount_gaps, gaps_time, avg_start, avg_end, created_at, updated_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
        )";

        for (const auto& schedule : schedules) {
            QVariantList values;
            values << setId
                   << schedule.index
                   << QString::fromStdString(DatabaseJsonHelpers::scheduleToJson(schedule))
                   << schedule.amount_days
                   << schedule.amount_gaps
                   << schedule.gaps_time
                   << schedule.avg_start
                   << schedule.avg_end;

            batchData.push_back(values);
        }

        // Execute batch insert
        bool success = DatabaseUtils::executeBatch(db, insertQuery, batchData);

        // Restore normal database settings
        DatabaseUtils::restoreNormalSettings(db);

        if (success) {
            // Update schedule count in set
            updateScheduleSetCount(setId);
            Logger::get().logInfo("Bulk insert completed successfully");
        } else {
            Logger::get().logError("Bulk insert failed");
        }

        return success;

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during bulk insert: " + std::string(e.what()));
        DatabaseUtils::restoreNormalSettings(db);
        return false;
    }
}

vector<InformativeSchedule> DatabaseScheduleManager::getAllSchedules() {
    return {};
}

vector<InformativeSchedule> DatabaseScheduleManager::getSchedulesByMetrics(int maxDays, int maxGaps, int maxGapTime,
                                                                           int minAvgStart, int maxAvgStart,
                                                                           int minAvgEnd, int maxAvgEnd) {
    return {};
}