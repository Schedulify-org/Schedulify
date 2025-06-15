#ifndef DB_SCHEDULES_H
#define DB_SCHEDULES_H

#include "db_entities.h"
#include "model_interfaces.h"
#include <QSqlDatabase>
#include <vector>
#include <string>

using namespace std;

class DatabaseScheduleManager {
public:
    explicit DatabaseScheduleManager(QSqlDatabase& database);

    // Schedule CRUD operations
    bool insertSchedule(const InformativeSchedule& schedule, const vector<int>& courseIds);
    bool insertSchedules(const vector<InformativeSchedule>& schedules,
                         const vector<vector<int>>& allCourseIds);
    bool updateSchedule(int scheduleId, const InformativeSchedule& schedule);
    bool deleteSchedule(int scheduleId);
    bool deleteAllSchedules();

    // Schedule retrieval operations
    vector<InformativeSchedule> getAllSchedules();
    InformativeSchedule getScheduleById(int id);
    vector<InformativeSchedule> getSchedulesByIndex(int scheduleIndex);
    vector<InformativeSchedule> getSchedulesWithCourseIds(const vector<int>& courseIds);

    // Schedule utility operations
    bool scheduleExists(int scheduleId);
    int getScheduleCount();
    QDateTime getScheduleCreationTime(int scheduleId);
    vector<int> getCourseIdsForSchedule(int scheduleId);

    // Schedule metadata operations
    bool insertScheduleMetadata(int totalSchedules, const string& generationSettings);
    bool updateScheduleMetadata(int id, const string& status);
    vector<ScheduleMetadataEntity> getAllScheduleMetadata();
    ScheduleMetadataEntity getLatestScheduleMetadata();
    bool deleteAllScheduleMetadata();

    // Schedule statistics
    int getScheduleCountByDays(int amountDays);
    int getScheduleCountByGaps(int maxGaps);
    vector<InformativeSchedule> getSchedulesCreatedAfter(const QDateTime& dateTime);

private:
    QSqlDatabase& db;

    // Helper methods
    bool executeQuery(const QString& query, const QVariantList& params = QVariantList());
    InformativeSchedule createScheduleFromQuery(QSqlQuery& query);
    ScheduleMetadataEntity createScheduleMetadataFromQuery(QSqlQuery& query);

    // JSON conversion helpers
    string scheduleToJson(const InformativeSchedule& schedule);
    InformativeSchedule scheduleFromJson(const string& json, int id, int schedule_index,
                                         int amount_days, int amount_gaps, int gaps_time,
                                         int avg_start, int avg_end);
    string courseIdsToJson(const vector<int>& courseIds);
    vector<int> courseIdsFromJson(const string& json);
};

#endif // DB_SCHEDULES_H