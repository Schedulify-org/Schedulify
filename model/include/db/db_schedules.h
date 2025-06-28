#ifndef DB_SCHEDULES_H
#define DB_SCHEDULES_H

#include "db_entities.h"
#include "model_interfaces.h"
#include "db_json_helpers.h"
#include "db_utils.h"
#include "logger.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QSqlDatabase>
#include <vector>
#include <string>
#include <map>

using namespace std;

class DatabaseScheduleManager {
public:
    explicit DatabaseScheduleManager(QSqlDatabase& database);

    // Schedule CRUD operations
    bool insertSchedule(const InformativeSchedule& schedule, const string& scheduleName = "",
                        const vector<int>& sourceFileIds = {});
    bool insertSchedules(const vector<InformativeSchedule>& schedules, const string& setName = "",
                         const vector<int>& sourceFileIds = {});
    bool updateSchedule(int scheduleId, const InformativeSchedule& schedule);
    bool deleteSchedule(int scheduleId);
    bool deleteAllSchedules();
    bool deleteSchedulesBySetId(int setId);

    // Schedule retrieval operations
    vector<InformativeSchedule> getAllSchedules();
    InformativeSchedule getScheduleById(int id);
    vector<InformativeSchedule> getSchedulesByName(const string& name);
    vector<InformativeSchedule> getSchedulesBySetId(int setId);

    // Schedule filtering operations
    vector<InformativeSchedule> getSchedulesByMetrics(int maxDays = -1, int maxGaps = -1, int maxGapTime = -1,
                                                      int minAvgStart = -1,int maxAvgStart = -1, int minAvgEnd = -1,
                                                      int maxAvgEnd = -1);

    // Schedule set operations
    int createScheduleSet(const string& setName, const vector<int>& sourceFileIds = {});
    vector<ScheduleSetEntity> getAllScheduleSets();
    ScheduleSetEntity getScheduleSetById(int setId);
    bool updateScheduleSet(int setId, const string& setName);
    bool deleteScheduleSet(int setId);

    // Utility operations
    bool scheduleExists(int scheduleId);
    bool scheduleSetExists(int setId);
    int getScheduleCount();
    int getScheduleCountBySetId(int setId);
    int getScheduleSetCount();
    QDateTime getScheduleCreationTime(int scheduleId);

    // Statistics operations
    map<string, int> getScheduleStatistics();
    vector<InformativeSchedule> getRecentSchedules(int limit = 10);

    // Performance operations for bulk inserts
    bool insertSchedulesBulk(const vector<InformativeSchedule>& schedules, int setId);

private:
    QSqlDatabase& db;

    // Helper methods
    static InformativeSchedule createScheduleFromQuery(QSqlQuery& query);
    static ScheduleSetEntity createScheduleSetFromQuery(QSqlQuery& query);
    bool updateScheduleSetCount(int setId);
};

#endif // DB_SCHEDULES_H