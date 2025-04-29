#ifndef MAIN_INCLUDE_H
#define MAIN_INCLUDE_H

#include <string>
#include <vector>

using namespace std;

// In your model header file
enum class ModelOperation {
    GENERATE_COURSES,
    GET_COURSES,
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

class Model {
public:
    void* executeOperation(ModelOperation operation, const void* data, const string& path = "");

private:
    static vector<Course> generateCourses(const string& path);
    static vector<Schedule> generateSchedules(const vector<Course>& userInput);
    static void saveSchedule(const Schedule& schedule, const string& path);
    static void printSchedule(const Schedule& schedule);

    vector<Course> lastGeneratedCourses;
    vector<Schedule> lastGeneratedSchedules;
};

#endif //MAIN_INCLUDE_H
