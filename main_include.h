#ifndef MAIN_INCLUDE_H
#define MAIN_INCLUDE_H

#include <string>
#include <vector>

using namespace std;

enum class ModelOperation {
    GENERATE_COURSES,
    GENERATE_SCHEDULES,
    SAVE_SCHEDULE,
    PRINT_SCHEDULE
};

class Session {
public:
    int day_of_week;
    string start_time;
    string end_time;
    string building_number;
    string room_number;
};

class Course {
public:
    int id;
    string raw_id;
    string name;
    string teacher;
    vector<Session> Lectures;
    vector<Session> Tirgulim;
    vector<Session> labs;
};

struct CourseSelection {
    int courseId;
    const Session* lecture;
    const Session* tutorial; // nullptr if none
    const Session* lab;      // nullptr if none
};

struct Schedule {
    vector<CourseSelection> selections;
};

struct CourseInfo {
    string raw_id;
    string name;
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
    vector<ScheduleDay> week;
};

class Model {
public:
    void* executeOperation(ModelOperation operation, const void* data, const string& path = "");

private:
    static vector<Course> generateCourses(const string& path);
    static vector<InformativeSchedule> generateSchedules(const vector<Course>& userInput, const vector<Course>& allCourses);
    static void saveSchedule(const InformativeSchedule& infoSchedule, const string& path);
    static void printSchedule(const InformativeSchedule& infoSchedule);

    vector<Course> lastGeneratedCourses;
    vector<InformativeSchedule> lastGeneratedSchedules;
};

#endif //MAIN_INCLUDE_H
