// TimeUtils.cpp
#include "schedule_algorithm/TimeUtils.h"
#include "schedule_algorithm/schedule_types.h"
#include <string>

using namespace std;    
int TimeUtils::toMinutes(const std::string& t) {
    size_t colonPos = t.find(':');
    if (colonPos == std::string::npos) {
        throw std::invalid_argument("Missing colon in time string: " + t);
    }

    std::string hourStr = t.substr(0, colonPos);
    std::string minuteStr = t.substr(colonPos + 1);

    if (hourStr.empty() || minuteStr.empty()) {
        throw std::invalid_argument("Empty hour or minute in time string: " + t);
    }

    int hours = std::stoi(hourStr);
    int minutes = std::stoi(minuteStr);

    if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
        throw std::invalid_argument("Hour or minute out of range: " + t);
    }

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