#ifndef POST_PARSER_H
#define POST_PARSER_H

#include "schedule_algorithm/ScheduleBuilder.h"

#include <algorithm>

struct ScheduleItem {
    string courseName;
    string raw_id;
    string type;
    string start;
    string end;
    string building;
    string room;

    string toJson() const;
};

// convert schedule to json file
void exportCompactJson(const vector<Schedule>& schedules, const string& outputPath);
void exportSchedulesByDayJson(const vector<Schedule>& schedules, const string& outputPath, const vector<Course>& courses);
void exportSchedulesByDayText(const vector<Schedule>& schedules, const string& outputPath, const vector<Course>& courses);

#endif //POST_PARSER_H
