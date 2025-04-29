#ifndef MAIN_MODEL_H
#define MAIN_MODEL_H

#include "parsers/parseCoursesToVector.h"
#include "schedule_algorithm/ScheduleBuilder.h"

#include <vector>
#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::vector;

// In your model header file
enum class ModelOperation {
    GENERATE_COURSES,
    GENERATE_SCHEDULES,
    SAVE_SCHEDULE,
    PRINT_SCHEDULE
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

#endif //MAIN_MODEL_H
