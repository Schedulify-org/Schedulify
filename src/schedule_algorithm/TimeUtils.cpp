// TimeUtils.cpp
#include "schedule_algorithm/TimeUtils.h"
#include "schedule_algorithm/schedule_types.h"
#include <string>

using namespace std;    
int TimeUtils::toMinutes(const string& t) {
    int colonPos = t.find(':');
    int hours = stoi(t.substr(0, colonPos));
    int minutes = stoi(t.substr(colonPos + 1));
    return hours * 60 + minutes;
}

bool TimeUtils::isOverlap(const Session* s1, const Session* s2) {
    if (s1->day_of_week != s2->day_of_week) return false;
    int start1 = toMinutes(s1->start_time);
    int end1 = toMinutes(s1->end_time);
    int start2 = toMinutes(s2->start_time);
    int end2 = toMinutes(s2->end_time);
    return (start1 < end2 && start2 < end1);
}