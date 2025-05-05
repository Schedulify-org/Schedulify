#ifndef PARSE_SCHED_TO_JSON
#define PARSE_SCHED_TO_JSON

#include "main_include.h"
#include "schedule_algorithm/ScheduleBuilder.h"

#include <algorithm>
#include <iomanip>

// convert schedule vector to json file
bool exportSchedulesToJson(const vector<Schedule>& schedules, const string& outputPath, const vector<Course>& courses);

// convert schedule item to text file
string ScheduleItemToJson(const ScheduleItem& s);

// Converts a day number (1-7) to its lowercase string name (Sunday = 1).
string dayToString(int day);

void writeDayScheduleToJson(ostream& outFile, const unordered_map<int, vector<ScheduleItem>>& dayMap, int day);

#endif //PARSE_SCHED_TO_JSON
