#ifndef POST_PARSER_H
#define POST_PARSER_H

#include "schedule_algorithm/ScheduleBuilder.h"

#include <algorithm>
#include <iomanip>

struct ScheduleItem {
    string courseName;
    string raw_id;
    string type;
    string start;
    string end;
    string building;
    string room;
};

struct CourseInfo {
    string raw_id;
    string name;
};

// convert schedule to json file
void exportSchedulesToJson(const vector<Schedule>& schedules, const string& outputPath, const vector<Course>& courses);

// convert schedule to text file
void exportSchedulesToText(const vector<Schedule>& schedules, const string& outputPath, const vector<Course>& courses);

// convert schedule item to text file
string ScheduleItemToJson(const ScheduleItem& s);

// Converts a day number (1-7) to its lowercase string name (Sunday = 1).
string dayToString(int day);

// get course name and id from course list
CourseInfo getCourseInfoById(const vector<Course>& courses, int courseId);

// Adds a session (lecture/tutorial/lab) to the day map if it exists.
void addSessionToDayMap(unordered_map<int, vector<ScheduleItem>>& dayMap, const Session* session, const string& type,
        const string& courseName, const string& raw_id);

// Builds a map from day index to list of scheduled items for a given schedule.
unordered_map<int, vector<ScheduleItem>> buildDayMapForSchedule(const Schedule& schedule, const vector<Course>& courses);

// Writes the list of scheduled items for a specific day to the output file.
void writeDayScheduleToFile(ostream& out,int day,const vector<ScheduleItem>& items);

// Writes a full schedule (including all days) to the output file.
void writeScheduleToFile(ostream& out, const Schedule& schedule, size_t index, const vector<Course>& courses);

#endif //POST_PARSER_H
