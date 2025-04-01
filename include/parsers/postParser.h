#ifndef POST_PARSER_H
#define POST_PARSER_H

#include "schedule_algorithm/ScheduleBuilder.h"

// convert schedule to json file
void exportCompactJson(const vector<Schedule>& schedules, const string& outputPath);

#endif //POST_PARSER_H
