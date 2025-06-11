#ifndef SCHEDULE_TO_MT_H
#define SCHEDULE_TO_MT_H

#include "model_interfaces.h"

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

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
    int longest_gap = 0;
};

struct ScheduleMeta {
    int index;

    // Basic fields from InformativeSchedule
    int amount_days;
    int amount_gaps;
    int gaps_time;
    int avg_start;
    int avg_end;

    // Calculated summary fields
    int total_minutes;
    int total_sessions;
    int max_sessions_per_day;
    int days_with_single_session;
    int earliest_start = 1440;
    int latest_end = 0;
    int longest_gap = 0;
    int blocked_time_minutes;

    // New advanced metrics
    double workload_variance;      // Variance in daily workload
    int free_days;                 // Number of days with no sessions

    // Boolean characteristics for easy filtering
    bool has_long_gap;            // Has gaps > 2 hours
    bool has_morning_session;     // Has sessions before 9 AM
    bool has_late_session;        // Has sessions after 6 PM
    bool has_midweek_break;       // No Wednesday classes
    bool has_back_to_back;        // Has sessions with ≤15 min gap
    bool is_balanced;             // Low variance in daily workload
    bool is_compact;              // High ratio of class time to blocked time

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
int calculateDayGaps(const ScheduleDay& day);
DayMeta calculateDayMeta(const ScheduleDay& day);
std::vector<CourseMeta> calculateCourseMeta(const InformativeSchedule& schedule);
bool hasMidweekBreak(const InformativeSchedule& schedule);
bool hasBackToBackSessions(const InformativeSchedule& schedule);
double calculateWorkloadVariance(const InformativeSchedule& schedule);
int calculateBlockedTime(const InformativeSchedule& schedule);
ScheduleMeta convertToScheduleMeta(const InformativeSchedule& schedule);
std::string scheduleMetaToJson(const ScheduleMeta& meta);

#endif // SCHEDULE_TO_MT_H