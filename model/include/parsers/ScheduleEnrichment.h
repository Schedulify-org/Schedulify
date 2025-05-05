#ifndef SCHEDULE_ENRICHMENT_H
#define SCHEDULE_ENRICHMENT_H

#include "main_include.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iostream>

using std::string;
using std::cout;
using std::vector;

enum class exportType {
    JSON,
    VECTOR
};

vector<InformativeSchedule> exportSchedulesToObjects(const vector<Schedule>& schedules, const vector<Course>& courses);

// filter the valid schedules out of the schedule vector
vector<Schedule> filterValidSchedules(const vector<Schedule>& schedules, const vector<Course>& courses);

// Adds a session (lecture/tutorial/lab) to the day map if it exists.
void addSessionToDayMap(unordered_map<int, vector<ScheduleItem>>& dayMap, const Session* session, const string& type,
                        const string& courseName, const string& raw_id);

// get course name and id from course list
CourseInfo getCourseInfoById(const vector<Course>& courses, int courseId, exportType type);

// Builds a map from day index to list of scheduled items for a given schedule.
unordered_map<int, vector<ScheduleItem>> buildDayMapForSchedule(const Schedule& schedule, const vector<Course>& courses,
                                                                exportType type);

// Converts a day number (1-7) to its lowercase string name (Sunday = 1).
string dayToString(int day);

// Print to console a vector of informative schedules
void printInformativeSchedules(const vector<InformativeSchedule>& schedules);


#endif //SCHEDULE_ENRICHMENT_H
