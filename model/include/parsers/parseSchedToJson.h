#ifndef PARSE_SCHED_TO_JSON
#define PARSE_SCHED_TO_JSON

#include "schedule_algorithm/ScheduleBuilder.h"
#include "parsers/ScheduleEnrichment.h"
#include "model_interfaces.h"
#include "logger/logger.h"

#include <iomanip>

// convert schedule vector to json file
bool exportSchedulesToJson(const vector<Schedule>& schedules, const string& outputPath, const vector<Course>& courses);

// convert schedule item to text file
string ScheduleItemToJson(const ScheduleItem& s);

void writeDayScheduleToJson(ostream& outFile, const unordered_map<int, vector<ScheduleItem>>& dayMap, int day);

#endif //PARSE_SCHED_TO_JSON
