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

enum class ModelOperation {
    GENERATE_COURSES,
    VALIDATE_COURSES,
    GENERATE_SCHEDULES,
    SAVE_SCHEDULE,
    PRINT_SCHEDULE,
    BOT_MESSAGE
};

class IModel {
public:
    virtual ~IModel() = default;
    virtual void* executeOperation(ModelOperation operation, const void* data, const std::string& path) = 0;
};

#endif //MODEL_INTERFACES_H
