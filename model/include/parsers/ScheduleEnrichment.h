#ifndef SCHEDULE_ENRICHMENT_H
#define SCHEDULE_ENRICHMENT_H

#include "model_interfaces.h"
#include "inner_structs.h"
#include "logger.h"

#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

using std::string;
using std::cout;
using std::vector;
using std::unordered_map;

// Converts schedules to InformativeSchedule objects.
vector<InformativeSchedule> exportSchedulesToObjects(const vector<Schedule>& schedules,
        const vector<Course>& courses);

// Creates a mapping from course IDs to CourseInfo objects.
unordered_map<int, CourseInfo> buildCourseInfoMap(const vector<Course>& courses);

// Adds a session to the day map if it exists.
void addSessionToDayMap(unordered_map<int, vector<ScheduleItem>>& dayMap, const Session* session,
        const string& sessionType,
        const CourseInfo& courseInfo);

// Builds a day map for a schedule.
unordered_map<int, std::vector<ScheduleItem>> buildDayMap(const Schedule& schedule,
        const unordered_map<int, CourseInfo>& courseInfoMap);


// checks if a schedule is valid.
bool isScheduleValid(const unordered_map<int, vector<ScheduleItem>>& dayMap);

// Creates a ScheduleDay object for a specific day.
ScheduleDay createScheduleDay(int day, unordered_map<int, vector<ScheduleItem>>& dayMap);

// Creates an InformativeSchedule from a Schedule.
InformativeSchedule createInformativeSchedule(const Schedule& schedule, size_t index,
        const unordered_map<int, CourseInfo>& courseInfoMap);

// Converts a day number to a string representation.
string dayToString(int day);

// Prints informative schedules to the console.
void printInformativeSchedules(const vector<InformativeSchedule>& schedules);


#endif //SCHEDULE_ENRICHMENT_H
