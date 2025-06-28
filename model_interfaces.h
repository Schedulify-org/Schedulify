#ifndef MODEL_INTERFACES_H
#define MODEL_INTERFACES_H

#include <string>
#include <vector>

using namespace std;

enum class SessionType {
    LECTURE,
    TUTORIAL,
    LAB,
    BLOCK,
    UNSUPPORTED
};

class Session {
public:
    int day_of_week;
    string start_time;
    string end_time;
    string building_number;
    string room_number;
};

class Group {
public:
    SessionType type;
    vector<Session> sessions;
};

class Course {
public:
    int id;
    string raw_id;
    string name;
    string teacher;
    vector<Group> Lectures;
    vector<Group> Tirgulim;
    vector<Group> labs;
    vector<Group> blocks;
};

struct ScheduleItem {
    string courseName;
    string raw_id;
    string type;
    string start;
    string end;
    string building;
    string room;
};

struct ScheduleDay {
    string day;
    vector<ScheduleItem> day_items;
};

struct InformativeSchedule {
    int index;
    int amount_days = 0;
    int amount_gaps = 0;
    int gaps_time = 0;
    int avg_start = 0;
    int avg_end = 0;
    vector<ScheduleDay> week;
};

struct FileLoadData {
    vector<int> fileIds;
    string operation_type;
    string filePath;
};

enum class ModelOperation {
    GENERATE_COURSES,
    VALIDATE_COURSES,
    GENERATE_SCHEDULES,
    SAVE_SCHEDULE,
    PRINT_SCHEDULE,
    LOAD_COURSES_FROM_DB,
    CLEAR_DATABASE,
    BOT_MESSAGE,
    BACKUP_DATABASE,
    RESTORE_DATABASE,
    GET_DATABASE_STATS,
    LOAD_FROM_HISTORY,
    GET_FILE_HISTORY,
    DELETE_FILE_FROM_HISTORY,
    SAVE_SCHEDULES_TO_DB,
    LOAD_SCHEDULES_FROM_DB,
    GET_SCHEDULE_SETS,
    DELETE_SCHEDULE_SET,
    GET_SCHEDULES_BY_SET_ID,
    FILTER_SCHEDULES_BY_METRICS,
    GET_SCHEDULE_STATISTICS
};

struct ScheduleFilterData {
    int maxDays = -1;
    int maxGaps = -1;
    int maxGapTime = -1;
    int minAvgStart = -1;
    int maxAvgStart = -1;
    int minAvgEnd = -1;
    int maxAvgEnd = -1;
    int setId = -1;  // Optional: filter by specific set

    ScheduleFilterData() = default;

    ScheduleFilterData(int maxD, int maxG, int maxGT, int minAS, int maxAS, int minAE, int maxAE)
            : maxDays(maxD), maxGaps(maxG), maxGapTime(maxGT),
              minAvgStart(minAS), maxAvgStart(maxAS), minAvgEnd(minAE), maxAvgEnd(maxAE) {}
};

struct ScheduleSaveData {
    vector<InformativeSchedule> schedules;
    string setName;
    vector<int> sourceFileIds;

    ScheduleSaveData() = default;

    ScheduleSaveData(const vector<InformativeSchedule>& scheds, const string& name, const vector<int>& fileIds)
            : schedules(scheds), setName(name), sourceFileIds(fileIds) {}
};

class IModel {
public:
    virtual ~IModel() = default;
    virtual void* executeOperation(ModelOperation operation, const void* data, const std::string& path) = 0;
};

#endif //MODEL_INTERFACES_H