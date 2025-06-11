#ifndef SCHEDULE_TO_MT_H
#define SCHEDULE_TO_MT_H

#include "model_interfaces.h"

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>


struct CourseMeta {
    std::string raw_id;
    std::string name;
    int lectures = 0;
    int tutorials = 0;
    int labs = 0;
    int earliest_start = 1440;
    int latest_end = 0;
};

struct DayMeta {
    std::string day;
    int num_sessions = 0;
    int total_minutes = 0;
    int earliest_start = 1440;
    int latest_end = 0;
};

struct ScheduleMeta {
    int index;

    // Summary fields
    int amount_days;
    int amount_gaps;
    int gaps_time;
    int avg_start;
    int avg_end;
    int total_minutes;
    int total_sessions;
    int max_sessions_per_day;
    int days_with_single_session;

    // Time constraints
    int blocked_time_minutes;

    // Boolean traits
    bool has_long_gap;
    bool has_morning_session;
    bool has_late_session;
    bool has_midweek_break;
    // Detailed breakdown
    std::vector<DayMeta> week;
    std::vector<CourseMeta> courses;
};

// Main function
std::string CalculateMetaData(const std::vector<InformativeSchedule>& schedules);

// Helper functions
int timeToMinutes(const std::string& time);
std::string minutesToTime(int minutes);
std::string escapeJson(const std::string& str);
DayMeta calculateDayMeta(const ScheduleDay& day);
std::vector<CourseMeta> calculateCourseMeta(const InformativeSchedule& schedule);
bool hasMidweekBreak(const InformativeSchedule& schedule);
int calculateBlockedTime(const InformativeSchedule& schedule);
ScheduleMeta convertToScheduleMeta(const InformativeSchedule& schedule);
std::string scheduleMetaToJson(const ScheduleMeta& meta);

#endif // SCHEDULE_TO_MT_H