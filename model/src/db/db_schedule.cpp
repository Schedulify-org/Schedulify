#include "db_schedules.h"
#include "db_json_helpers.h"
#include "logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

DatabaseScheduleManager::DatabaseScheduleManager(QSqlDatabase& database) : db(database) {
}

bool DatabaseScheduleManager::insertSchedule(const InformativeSchedule& schedule, const vector<int>& courseIds) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule insertion");
        return false;
    }

    // Convert course IDs to JSON
    string courseJson = courseIdsToJson(courseIds);

    // Convert schedule week to JSON
    string weekJson = scheduleToJson(schedule);

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO schedule
        (schedule_index, courses_json, week_json, amount_days, amount_gaps, gaps_time, avg_start, avg_end)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(schedule.index);
    query.addBindValue(QString::fromStdString(courseJson));
    query.addBindValue(QString::fromStdString(weekJson));
    query.addBindValue(schedule.amount_days);
    query.addBindValue(schedule.amount_gaps);
    query.addBindValue(schedule.gaps_time);
    query.addBindValue(schedule.avg_start);
    query.addBindValue(schedule.avg_end);

    if (!query.exec()) {
        Logger::get().logError("Failed to insert schedule: " + query.lastError().text().toStdString());
        Logger::get().logError("Schedule index: " + std::to_string(schedule.index));
        return false;
    }

    Logger::get().logInfo("Successfully inserted schedule with index: " + std::to_string(schedule.index));
    return true;
}

bool DatabaseScheduleManager::insertSchedules(const vector<InformativeSchedule>& schedules,
                                              const vector<vector<int>>& allCourseIds) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule insertion");
        return false;
    }

    if (schedules.size() != allCourseIds.size()) {
        Logger::get().logError("Schedules and course IDs vectors size mismatch");
        return false;
    }

    if (schedules.empty()) {
        Logger::get().logWarning("No schedules to insert");
        return true;
    }

    // Start transaction
    if (!db.transaction()) {
        Logger::get().logError("Failed to begin transaction for schedule insertion");
        return false;
    }

    int successCount = 0;
    for (size_t i = 0; i < schedules.size(); ++i) {
        if (insertSchedule(schedules[i], allCourseIds[i])) {
            successCount++;
        } else {
            Logger::get().logError("Failed to insert schedule with index: " + std::to_string(schedules[i].index));
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

    Logger::get().logInfo("Successfully inserted " + std::to_string(successCount) + "/" +
                          std::to_string(schedules.size()) + " schedules");

    return successCount == static_cast<int>(schedules.size());
}

bool DatabaseScheduleManager::updateSchedule(int scheduleId, const InformativeSchedule& schedule) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule update");
        return false;
    }

    if (scheduleId <= 0) {
        Logger::get().logError("Invalid schedule ID for update: " + std::to_string(scheduleId));
        return false;
    }

    string weekJson = scheduleToJson(schedule);

    QSqlQuery query(db);
    query.prepare(R"(
        UPDATE schedule SET
        schedule_index = ?, week_json = ?, amount_days = ?, amount_gaps = ?,
        gaps_time = ?, avg_start = ?, avg_end = ?
        WHERE id = ?
    )");

    query.addBindValue(schedule.index);
    query.addBindValue(QString::fromStdString(weekJson));
    query.addBindValue(schedule.amount_days);
    query.addBindValue(schedule.amount_gaps);
    query.addBindValue(schedule.gaps_time);
    query.addBindValue(schedule.avg_start);
    query.addBindValue(schedule.avg_end);
    query.addBindValue(scheduleId);

    if (!query.exec()) {
        Logger::get().logError("Failed to update schedule ID " + std::to_string(scheduleId) +
                               ": " + query.lastError().text().toStdString());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    if (rowsAffected == 0) {
        Logger::get().logWarning("No schedule found with ID " + std::to_string(scheduleId) + " for update");
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

    if (scheduleId <= 0) {
        Logger::get().logError("Invalid schedule ID for deletion: " + std::to_string(scheduleId));
        return false;
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

vector<InformativeSchedule> DatabaseScheduleManager::getAllSchedules() {
    vector<InformativeSchedule> schedules;
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule retrieval");
        return schedules;
    }

    QSqlQuery query(R"(
        SELECT id, schedule_index, week_json, amount_days, amount_gaps, gaps_time, avg_start, avg_end
        FROM schedule ORDER BY schedule_index
    )", db);

    if (!query.exec()) {
        Logger::get().logError("Failed to retrieve schedules: " + query.lastError().text().toStdString());
        return schedules;
    }

    while (query.next()) {
        try {
            InformativeSchedule schedule = createScheduleFromQuery(query);
            schedules.push_back(schedule);
        } catch (const std::exception& e) {
            Logger::get().logError("Error parsing schedule data: " + string(e.what()));
            continue;
        }
    }

    Logger::get().logInfo("Retrieved " + std::to_string(schedules.size()) + " schedules from database");
    return schedules;
}

InformativeSchedule DatabaseScheduleManager::getScheduleById(int id) {
    InformativeSchedule schedule;
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule retrieval");
        return schedule;
    }

    if (id <= 0) {
        Logger::get().logError("Invalid schedule ID for retrieval: " + std::to_string(id));
        return schedule;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, schedule_index, week_json, amount_days, amount_gaps, gaps_time, avg_start, avg_end
        FROM schedule WHERE id = ?
    )");
    query.addBindValue(id);

    if (!query.exec()) {
        Logger::get().logError("Failed to retrieve schedule by ID " + std::to_string(id) +
                               ": " + query.lastError().text().toStdString());
        return schedule;
    }

    if (query.next()) {
        try {
            schedule = createScheduleFromQuery(query);
            Logger::get().logInfo("Retrieved schedule with ID: " + std::to_string(id));
        } catch (const std::exception& e) {
            Logger::get().logError("Error parsing schedule data for ID " + std::to_string(id) +
                                   ": " + string(e.what()));
        }
    } else {
        Logger::get().logWarning("No schedule found with ID: " + std::to_string(id));
    }

    return schedule;
}

vector<InformativeSchedule> DatabaseScheduleManager::getSchedulesByIndex(int scheduleIndex) {
    vector<InformativeSchedule> schedules;
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule retrieval");
        return schedules;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, schedule_index, week_json, amount_days, amount_gaps, gaps_time, avg_start, avg_end
        FROM schedule WHERE schedule_index = ? ORDER BY id
    )");
    query.addBindValue(scheduleIndex);

    if (!query.exec()) {
        Logger::get().logError("Failed to retrieve schedules by index " + std::to_string(scheduleIndex) +
                               ": " + query.lastError().text().toStdString());
        return schedules;
    }

    while (query.next()) {
        try {
            InformativeSchedule schedule = createScheduleFromQuery(query);
            schedules.push_back(schedule);
        } catch (const std::exception& e) {
            Logger::get().logError("Error parsing schedule data for index " + std::to_string(scheduleIndex) +
                                   ": " + string(e.what()));
            continue;
        }
    }

    Logger::get().logInfo("Retrieved " + std::to_string(schedules.size()) +
                          " schedules with index: " + std::to_string(scheduleIndex));
    return schedules;
}

vector<InformativeSchedule> DatabaseScheduleManager::getSchedulesWithCourseIds(const vector<int>& courseIds) {
    vector<InformativeSchedule> schedules;
    if (!db.isOpen() || courseIds.empty()) {
        Logger::get().logError("Database not open or no course IDs provided");
        return schedules;
    }

    vector<InformativeSchedule> allSchedules = getAllSchedules();

    for (const auto& schedule : allSchedules) {
        vector<int> scheduleCourseIds = getCourseIdsForSchedule(schedule.index);

        // Check if any of the requested course IDs are in this schedule
        bool hasMatchingCourse = false;
        for (int courseId : courseIds) {
            if (std::find(scheduleCourseIds.begin(), scheduleCourseIds.end(), courseId) != scheduleCourseIds.end()) {
                hasMatchingCourse = true;
                break;
            }
        }

        if (hasMatchingCourse) {
            schedules.push_back(schedule);
        }
    }

    Logger::get().logInfo("Found " + std::to_string(schedules.size()) +
                          " schedules containing requested courses");
    return schedules;
}

bool DatabaseScheduleManager::scheduleExists(int scheduleId) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule existence check");
        return false;
    }

    if (scheduleId <= 0) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM schedule WHERE id = ?");
    query.addBindValue(scheduleId);

    if (!query.exec()) {
        Logger::get().logError("Failed to check schedule existence for ID " + std::to_string(scheduleId) +
                               ": " + query.lastError().text().toStdString());
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

int DatabaseScheduleManager::getScheduleCount() {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule count");
        return -1;
    }

    QSqlQuery query("SELECT COUNT(*) FROM schedule", db);

    if (!query.exec()) {
        Logger::get().logError("Failed to get schedule count: " + query.lastError().text().toStdString());
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    Logger::get().logError("Unexpected error getting schedule count");
    return -1;
}

QDateTime DatabaseScheduleManager::getScheduleCreationTime(int scheduleId) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule creation time retrieval");
        return QDateTime();
    }

    if (scheduleId <= 0) {
        Logger::get().logError("Invalid schedule ID for creation time: " + std::to_string(scheduleId));
        return QDateTime();
    }

    QSqlQuery query(db);
    query.prepare("SELECT created_at FROM schedule WHERE id = ?");
    query.addBindValue(scheduleId);

    if (!query.exec()) {
        Logger::get().logError("Failed to get creation time for schedule ID " + std::to_string(scheduleId) +
                               ": " + query.lastError().text().toStdString());
        return QDateTime();
    }

    if (query.next()) {
        QDateTime creationTime = query.value(0).toDateTime();
        if (!creationTime.isValid()) {
            Logger::get().logWarning("Invalid creation time found for schedule ID: " + std::to_string(scheduleId));
        }
        return creationTime;
    }

    Logger::get().logWarning("No creation time found for schedule ID: " + std::to_string(scheduleId));
    return QDateTime();
}

vector<int> DatabaseScheduleManager::getCourseIdsForSchedule(int scheduleId) {
    vector<int> courseIds;
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for course IDs retrieval");
        return courseIds;
    }

    QSqlQuery query(db);
    query.prepare("SELECT courses_json FROM schedule WHERE id = ?");
    query.addBindValue(scheduleId);

    if (!query.exec()) {
        Logger::get().logError("Failed to get course IDs for schedule ID " + std::to_string(scheduleId) +
                               ": " + query.lastError().text().toStdString());
        return courseIds;
    }

    if (query.next()) {
        string coursesJson = query.value(0).toString().toStdString();
        courseIds = courseIdsFromJson(coursesJson);
    }

    return courseIds;
}

bool DatabaseScheduleManager::insertScheduleMetadata(int totalSchedules, const string& generationSettings) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule metadata insertion");
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO schedule_metadata (total_schedules, generation_settings_json)
        VALUES (?, ?)
    )");

    query.addBindValue(totalSchedules);
    query.addBindValue(QString::fromStdString(generationSettings));

    if (!query.exec()) {
        Logger::get().logError("Failed to insert schedule metadata: " + query.lastError().text().toStdString());
        return false;
    }

    Logger::get().logInfo("Successfully inserted schedule metadata for " + std::to_string(totalSchedules) + " schedules");
    return true;
}

bool DatabaseScheduleManager::updateScheduleMetadata(int id, const string& status) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule metadata update");
        return false;
    }

    if (id <= 0) {
        Logger::get().logError("Invalid schedule metadata ID for update: " + std::to_string(id));
        return false;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE schedule_metadata SET status = ? WHERE id = ?");
    query.addBindValue(QString::fromStdString(status));
    query.addBindValue(id);

    if (!query.exec()) {
        Logger::get().logError("Failed to update schedule metadata ID " + std::to_string(id) +
                               ": " + query.lastError().text().toStdString());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    if (rowsAffected == 0) {
        Logger::get().logWarning("No schedule metadata found with ID " + std::to_string(id) + " for update");
        return false;
    }

    Logger::get().logInfo("Schedule metadata updated successfully (ID: " + std::to_string(id) + ")");
    return true;
}

vector<ScheduleMetadataEntity> DatabaseScheduleManager::getAllScheduleMetadata() {
    vector<ScheduleMetadataEntity> metadata;
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule metadata retrieval");
        return metadata;
    }

    QSqlQuery query(R"(
        SELECT id, total_schedules, generation_settings_json, generated_at, status
        FROM schedule_metadata ORDER BY generated_at DESC
    )", db);

    if (!query.exec()) {
        Logger::get().logError("Failed to retrieve schedule metadata: " + query.lastError().text().toStdString());
        return metadata;
    }

    while (query.next()) {
        try {
            ScheduleMetadataEntity entity = createScheduleMetadataFromQuery(query);
            metadata.push_back(entity);
        } catch (const std::exception& e) {
            Logger::get().logError("Error parsing schedule metadata: " + string(e.what()));
            continue;
        }
    }

    Logger::get().logInfo("Retrieved " + std::to_string(metadata.size()) + " schedule metadata entries");
    return metadata;
}

ScheduleMetadataEntity DatabaseScheduleManager::getLatestScheduleMetadata() {
    ScheduleMetadataEntity entity;
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for latest schedule metadata retrieval");
        return entity;
    }

    QSqlQuery query(R"(
        SELECT id, total_schedules, generation_settings_json, generated_at, status
        FROM schedule_metadata ORDER BY generated_at DESC LIMIT 1
    )", db);

    if (!query.exec()) {
        Logger::get().logError("Failed to retrieve latest schedule metadata: " + query.lastError().text().toStdString());
        return entity;
    }

    if (query.next()) {
        try {
            entity = createScheduleMetadataFromQuery(query);
            Logger::get().logInfo("Retrieved latest schedule metadata (ID: " + std::to_string(entity.id) + ")");
        } catch (const std::exception& e) {
            Logger::get().logError("Error parsing latest schedule metadata: " + string(e.what()));
        }
    } else {
        Logger::get().logInfo("No schedule metadata found in database");
    }

    return entity;
}

bool DatabaseScheduleManager::deleteAllScheduleMetadata() {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule metadata deletion");
        return false;
    }

    QSqlQuery query("DELETE FROM schedule_metadata", db);
    if (!query.exec()) {
        Logger::get().logError("Failed to delete all schedule metadata: " + query.lastError().text().toStdString());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    Logger::get().logInfo("Deleted all schedule metadata from database (" + std::to_string(rowsAffected) + " entries)");
    return true;
}

int DatabaseScheduleManager::getScheduleCountByDays(int amountDays) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule count by days");
        return -1;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM schedule WHERE amount_days = ?");
    query.addBindValue(amountDays);

    if (!query.exec()) {
        Logger::get().logError("Failed to get schedule count by days: " + query.lastError().text().toStdString());
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

int DatabaseScheduleManager::getScheduleCountByGaps(int maxGaps) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedule count by gaps");
        return -1;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM schedule WHERE amount_gaps <= ?");
    query.addBindValue(maxGaps);

    if (!query.exec()) {
        Logger::get().logError("Failed to get schedule count by gaps: " + query.lastError().text().toStdString());
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

vector<InformativeSchedule> DatabaseScheduleManager::getSchedulesCreatedAfter(const QDateTime& dateTime) {
    vector<InformativeSchedule> schedules;
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for schedules created after retrieval");
        return schedules;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, schedule_index, week_json, amount_days, amount_gaps, gaps_time, avg_start, avg_end
        FROM schedule WHERE created_at > ? ORDER BY created_at DESC
    )");
    query.addBindValue(dateTime);

    if (!query.exec()) {
        Logger::get().logError("Failed to retrieve schedules created after date: " + query.lastError().text().toStdString());
        return schedules;
    }

    while (query.next()) {
        try {
            InformativeSchedule schedule = createScheduleFromQuery(query);
            schedules.push_back(schedule);
        } catch (const std::exception& e) {
            Logger::get().logError("Error parsing schedule data for created after query: " + string(e.what()));
            continue;
        }
    }

    Logger::get().logInfo("Retrieved " + std::to_string(schedules.size()) + " schedules created after specified date");
    return schedules;
}

bool DatabaseScheduleManager::executeQuery(const QString& query, const QVariantList& params) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for query execution");
        return false;
    }

    QSqlQuery sqlQuery(db);
    sqlQuery.prepare(query);

    for (const QVariant& param : params) {
        sqlQuery.addBindValue(param);
    }

    if (!sqlQuery.exec()) {
        Logger::get().logError("Failed to execute query: " + sqlQuery.lastError().text().toStdString());
        Logger::get().logError("Query was: " + query.toStdString());
        return false;
    }

    return true;
}

InformativeSchedule DatabaseScheduleManager::createScheduleFromQuery(QSqlQuery& query) {
    InformativeSchedule schedule;

    // Extract values with validation
    int id = query.value(0).toInt();
    int schedule_index = query.value(1).toInt();
    string week_json = query.value(2).toString().toStdString();
    int amount_days = query.value(3).toInt();
    int amount_gaps = query.value(4).toInt();
    int gaps_time = query.value(5).toInt();
    int avg_start = query.value(6).toInt();
    int avg_end = query.value(7).toInt();

    if (week_json.empty()) {
        throw std::runtime_error("Empty week JSON in schedule query result");
    }

    schedule = scheduleFromJson(week_json, id, schedule_index, amount_days, amount_gaps, gaps_time, avg_start, avg_end);

    return schedule;
}

ScheduleMetadataEntity DatabaseScheduleManager::createScheduleMetadataFromQuery(QSqlQuery& query) {
    ScheduleMetadataEntity entity;

    entity.id = query.value(0).toInt();
    entity.total_schedules = query.value(1).toInt();
    entity.generation_settings_json = query.value(2).toString().toStdString();
    entity.generated_at = query.value(3).toDateTime();
    entity.status = query.value(4).toString().toStdString();

    if (entity.id <= 0) {
        throw std::runtime_error("Invalid schedule metadata ID in query result");
    }

    if (!entity.generated_at.isValid()) {
        Logger::get().logWarning("Invalid generated_at time for schedule metadata ID: " + std::to_string(entity.id));
        entity.generated_at = QDateTime::currentDateTime();
    }

    return entity;
}

// JSON conversion helpers
string DatabaseScheduleManager::scheduleToJson(const InformativeSchedule& schedule) {
    return DatabaseJsonHelpers::scheduleToJson(schedule);
}

InformativeSchedule DatabaseScheduleManager::scheduleFromJson(const string& json, int id, int schedule_index,
                                                              int amount_days, int amount_gaps, int gaps_time,
                                                              int avg_start, int avg_end) {
    return DatabaseJsonHelpers::scheduleFromJson(json, id, schedule_index, amount_days, amount_gaps, gaps_time, avg_start, avg_end);
}

string DatabaseScheduleManager::courseIdsToJson(const vector<int>& courseIds) {
    QJsonArray array;
    for (int id : courseIds) {
        array.append(id);
    }
    QJsonDocument doc(array);
    return doc.toJson(QJsonDocument::Compact).toStdString();
}

vector<int> DatabaseScheduleManager::courseIdsFromJson(const string& json) {
    vector<int> courseIds;

    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json));
    if (!doc.isArray()) {
        Logger::get().logWarning("Invalid course IDs JSON format");
        return courseIds;
    }

    QJsonArray array = doc.array();
    for (const auto& value : array) {
        if (value.isDouble()) {
            courseIds.push_back(value.toInt());
        }
    }

    return courseIds;
}