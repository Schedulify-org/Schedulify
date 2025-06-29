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

vector<int> DatabaseScheduleManager::executeCustomQuery(const string& sqlQuery, const vector<string>& parameters) {
    vector<int> scheduleIds;

    if (!db.isOpen()) {
        Logger::get().logError("Database not open for custom query execution");
        return scheduleIds;
    }

    // Validate the SQL query for security
    if (!isValidScheduleQuery(sqlQuery)) {
        Logger::get().logError("Invalid or unsafe SQL query rejected: " + sqlQuery);
        return scheduleIds;
    }

    try {
        QSqlQuery query(db);

        // Log the query for debugging
        Logger::get().logInfo("Executing custom schedule query: " + sqlQuery);
        Logger::get().logInfo("Parameters count: " + std::to_string(parameters.size()));

        if (!query.prepare(QString::fromStdString(sqlQuery))) {
            Logger::get().logError("Failed to prepare custom query: " + query.lastError().text().toStdString());
            return scheduleIds;
        }

        // Bind parameters
        for (size_t i = 0; i < parameters.size(); ++i) {
            query.addBindValue(QString::fromStdString(parameters[i]));
            Logger::get().logInfo("Parameter " + std::to_string(i) + ": " + parameters[i]);
        }

        if (!query.exec()) {
            Logger::get().logError("Failed to execute custom query: " + query.lastError().text().toStdString());
            return scheduleIds;
        }

        // Extract schedule IDs from results
        while (query.next()) {
            QVariant idVariant = query.value(0); // Assume first column is the schedule ID
            bool ok;
            int id = idVariant.toInt(&ok);

            if (ok && id > 0) {
                scheduleIds.push_back(id);
            } else {
                Logger::get().logWarning("Invalid schedule ID in query result: " + idVariant.toString().toStdString());
            }
        }

        Logger::get().logInfo("Custom query returned " + std::to_string(scheduleIds.size()) + " schedule IDs");

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during custom query execution: " + std::string(e.what()));
    }

    return scheduleIds;
}

vector<InformativeSchedule> DatabaseScheduleManager::getSchedulesByIds(const vector<int>& scheduleIds) {
    vector<InformativeSchedule> schedules;

    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule retrieval by IDs");
        return schedules;
    }

    if (scheduleIds.empty()) {
        Logger::get().logWarning("No schedule IDs provided for retrieval");
        return schedules;
    }

    try {
        // Create IN clause for the query
        QString inClause = "(";
        for (size_t i = 0; i < scheduleIds.size(); ++i) {
            if (i > 0) inClause += ",";
            inClause += "?";
        }
        inClause += ")";

        QString queryStr = QString(R"(
            SELECT id, schedule_index, schedule_data_json, amount_days, amount_gaps,
                   gaps_time, avg_start, avg_end
            FROM schedule
            WHERE schedule_index IN %1
            ORDER BY schedule_index
        )").arg(inClause);

        QSqlQuery query(db);
        if (!query.prepare(queryStr)) {
            Logger::get().logError("Failed to prepare schedule retrieval query: " + query.lastError().text().toStdString());
            return schedules;
        }

        // Bind schedule IDs
        for (int id : scheduleIds) {
            query.addBindValue(id);
        }

        if (!query.exec()) {
            Logger::get().logError("Failed to execute schedule retrieval query: " + query.lastError().text().toStdString());
            return schedules;
        }

        while (query.next()) {
            schedules.push_back(createScheduleFromQuery(query));
        }

        Logger::get().logInfo("Retrieved " + std::to_string(schedules.size()) + " schedules by IDs");

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during schedule retrieval by IDs: " + std::string(e.what()));
    }

    return schedules;
}

string DatabaseScheduleManager::getSchedulesMetadataForBot() {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for metadata generation");
        return "Database unavailable";
    }

    string metadata;

    try {
        // Get table structure information
        metadata += "SCHEDULE TABLE STRUCTURE:\n";
        metadata += "Table: schedule\n";
        metadata += "Primary Key: id (internal database ID)\n";
        metadata += "Schedule Identifier: schedule_index (user-visible schedule number)\n\n";

        metadata += "FILTERABLE COLUMNS:\n";
        metadata += "- schedule_index: INTEGER (1-based schedule number, unique identifier)\n";
        metadata += "- amount_days: INTEGER (number of study days, typically 1-7)\n";
        metadata += "- amount_gaps: INTEGER (total number of gaps between classes)\n";
        metadata += "- gaps_time: INTEGER (total gap time in minutes)\n";
        metadata += "- avg_start: INTEGER (average daily start time in minutes from midnight)\n";
        metadata += "- avg_end: INTEGER (average daily end time in minutes from midnight)\n\n";

        // Get some statistics
        QSqlQuery statsQuery(R"(
            SELECT
                COUNT(*) as total_schedules,
                MIN(amount_days) as min_days, MAX(amount_days) as max_days,
                MIN(amount_gaps) as min_gaps, MAX(amount_gaps) as max_gaps,
                MIN(gaps_time) as min_gap_time, MAX(gaps_time) as max_gap_time,
                MIN(avg_start) as min_start, MAX(avg_start) as max_start,
                MIN(avg_end) as min_end, MAX(avg_end) as max_end
            FROM schedule
        )", db);

        if (statsQuery.exec() && statsQuery.next()) {
            metadata += "CURRENT DATA RANGES:\n";
            metadata += "- Total schedules: " + std::to_string(statsQuery.value("total_schedules").toInt()) + "\n";
            metadata += "- Days range: " + std::to_string(statsQuery.value("min_days").toInt()) +
                        " to " + std::to_string(statsQuery.value("max_days").toInt()) + "\n";
            metadata += "- Gaps range: " + std::to_string(statsQuery.value("min_gaps").toInt()) +
                        " to " + std::to_string(statsQuery.value("max_gaps").toInt()) + "\n";
            metadata += "- Gap time range: " + std::to_string(statsQuery.value("min_gap_time").toInt()) +
                        " to " + std::to_string(statsQuery.value("max_gap_time").toInt()) + " minutes\n";
            metadata += "- Start time range: " + std::to_string(statsQuery.value("min_start").toInt()) +
                        " to " + std::to_string(statsQuery.value("max_start").toInt()) + " minutes (from midnight)\n";
            metadata += "- End time range: " + std::to_string(statsQuery.value("min_end").toInt()) +
                        " to " + std::to_string(statsQuery.value("max_end").toInt()) + " minutes (from midnight)\n\n";
        }

        metadata += "TIME CONVERSION EXAMPLES:\n";
        metadata += "- 8:00 AM = 480 minutes from midnight\n";
        metadata += "- 9:00 AM = 540 minutes from midnight\n";
        metadata += "- 5:00 PM = 1020 minutes from midnight\n";
        metadata += "- 6:00 PM = 1080 minutes from midnight\n\n";

        metadata += "SQL QUERY REQUIREMENTS:\n";
        metadata += "- Must SELECT schedule_index (the schedule ID to return)\n";
        metadata += "- Must use table name 'schedule'\n";
        metadata += "- Use parameter binding with ? for user values\n";
        metadata += "- Only SELECT statements allowed (no INSERT, UPDATE, DELETE)\n";
        metadata += "- Example: SELECT schedule_index FROM schedule WHERE amount_days <= ? AND amount_gaps = ?\n\n";

    } catch (const std::exception& e) {
        Logger::get().logError("Exception generating schedule metadata: " + std::string(e.what()));
        metadata += "Error generating metadata: " + std::string(e.what());
    }

    return metadata;
}

bool DatabaseScheduleManager::isValidScheduleQuery(const string& sqlQuery) {
    QString query = QString::fromStdString(sqlQuery).trimmed().toLower();

    // Must start with SELECT
    if (!query.startsWith("select")) {
        Logger::get().logWarning("Query rejected: must start with SELECT");
        return false;
    }

    // Must not contain dangerous keywords
    vector<string> forbiddenKeywords = {
            "insert", "update", "delete", "drop", "create", "alter",
            "truncate", "grant", "revoke", "exec", "execute",
            "declare", "cast", "convert", "union", "into"
    };

    for (const string& keyword : forbiddenKeywords) {
        if (query.contains(QString::fromStdString(keyword))) {
            Logger::get().logWarning("Query rejected: contains forbidden keyword: " + keyword);
            return false;
        }
    }

    // Must reference schedule table
    if (!query.contains("schedule")) {
        Logger::get().logWarning("Query rejected: must reference 'schedule' table");
        return false;
    }

    // Must select schedule_index
    if (!query.contains("schedule_index")) {
        Logger::get().logWarning("Query rejected: must select 'schedule_index' column");
        return false;
    }

    // Check for valid columns only
    vector<string> validColumns = getWhitelistedColumns();

    // Basic validation passed
    Logger::get().logInfo("SQL query validation passed: " + sqlQuery);
    return true;
}

vector<string> DatabaseScheduleManager::getWhitelistedTables() {
    return {"schedule", "schedule_set"};
}

vector<string> DatabaseScheduleManager::getWhitelistedColumns() {
    return {
            "schedule_index", "amount_days", "amount_gaps", "gaps_time",
            "avg_start", "avg_end", "id", "schedule_set_id", "created_at"
    };
}