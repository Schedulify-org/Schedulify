#include "scheduleToMT.h"

// Helper function to convert time string (HH:MM) to minutes from midnight
int timeToMinutes(const std::string& time) {
    if (time.empty()) return 0;

    size_t colonPos = time.find(':');
    if (colonPos == std::string::npos) return 0;

    int hours = std::stoi(time.substr(0, colonPos));
    int minutes = std::stoi(time.substr(colonPos + 1));

    return hours * 60 + minutes;
}

// Helper function to convert minutes to time string (HH:MM)
std::string minutesToTime(int minutes) {
    int hours = minutes / 60;
    int mins = minutes % 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setfill('0') << std::setw(2) << mins;
    return oss.str();
}

// Helper function to escape JSON strings
std::string escapeJson(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

// Helper function to calculate gaps within a day
int calculateDayGaps(const ScheduleDay& day) {
    if (day.day_items.size() <= 1) return 0;

    // Sort sessions by start time
    std::vector<ScheduleItem> sortedItems = day.day_items;
    std::sort(sortedItems.begin(), sortedItems.end(),
              [](const ScheduleItem& a, const ScheduleItem& b) {
                  return timeToMinutes(a.start) < timeToMinutes(b.start);
              });

    int maxGap = 0;
    for (size_t i = 1; i < sortedItems.size(); ++i) {
        int prevEnd = timeToMinutes(sortedItems[i-1].end);
        int currentStart = timeToMinutes(sortedItems[i].start);
        int gap = currentStart - prevEnd;
        maxGap = std::max(maxGap, gap);
    }

    return maxGap;
}

// Helper function to calculate DayMeta for a given day
DayMeta calculateDayMeta(const ScheduleDay& day) {
    DayMeta dayMeta;
    dayMeta.day = day.day;
    dayMeta.num_sessions = day.day_items.size();
    dayMeta.total_minutes = 0;
    dayMeta.earliest_start = 1440;
    dayMeta.latest_end = 0;
    dayMeta.longest_gap = 0;

    for (const auto& item : day.day_items) {
        int start = timeToMinutes(item.start);
        int end = timeToMinutes(item.end);

        dayMeta.total_minutes += (end - start);
        dayMeta.earliest_start = std::min(dayMeta.earliest_start, start);
        dayMeta.latest_end = std::max(dayMeta.latest_end, end);
    }

    // Calculate longest gap for this day
    dayMeta.longest_gap = calculateDayGaps(day);

    // If no sessions, reset earliest_start to 0
    if (dayMeta.num_sessions == 0) {
        dayMeta.earliest_start = 0;
    }

    return dayMeta;
}

// Helper function to calculate CourseMeta for all courses in a schedule
std::vector<CourseMeta> calculateCourseMeta(const InformativeSchedule& schedule) {
    std::map<std::string, CourseMeta> courseMap;

    for (const auto& day : schedule.week) {
        for (const auto& item : day.day_items) {
            auto& course = courseMap[item.raw_id];
            if (course.raw_id.empty()) {
                course.raw_id = item.raw_id;
                course.name = item.courseName;
            }

            // Count session types
            std::string lowerType = item.type;
            std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);

            if (lowerType.find("lecture") != std::string::npos || lowerType.find("lec") != std::string::npos) {
                course.lectures++;
            } else if (lowerType.find("tutorial") != std::string::npos || lowerType.find("tut") != std::string::npos) {
                course.tutorials++;
            } else if (lowerType.find("lab") != std::string::npos || lowerType.find("laboratory") != std::string::npos) {
                course.labs++;
            }

            // Update time constraints
            int start = timeToMinutes(item.start);
            int end = timeToMinutes(item.end);

            course.earliest_start = std::min(course.earliest_start, start);
            course.latest_end = std::max(course.latest_end, end);
        }
    }

    std::vector<CourseMeta> courses;
    for (const auto& pair : courseMap) {
        courses.push_back(pair.second);
    }

    return courses;
}

// Helper function to check if there's a midweek break (no classes on Wednesday)
bool hasMidweekBreak(const InformativeSchedule& schedule) {
    for (const auto& day : schedule.week) {
        if (day.day == "Wednesday" || day.day == "Wed") {
            return day.day_items.empty();
        }
    }
    return false;
}

// Helper function to check for back-to-back sessions
bool hasBackToBackSessions(const InformativeSchedule& schedule) {
    for (const auto& day : schedule.week) {
        if (day.day_items.size() <= 1) continue;

        std::vector<ScheduleItem> sortedItems = day.day_items;
        std::sort(sortedItems.begin(), sortedItems.end(),
                  [](const ScheduleItem& a, const ScheduleItem& b) {
                      return timeToMinutes(a.start) < timeToMinutes(b.start);
                  });

        for (size_t i = 1; i < sortedItems.size(); ++i) {
            int prevEnd = timeToMinutes(sortedItems[i-1].end);
            int currentStart = timeToMinutes(sortedItems[i].start);
            if (currentStart - prevEnd <= 15) { // 15 minutes or less gap
                return true;
            }
        }
    }
    return false;
}

// Helper function to calculate workload distribution variance
double calculateWorkloadVariance(const InformativeSchedule& schedule) {
    std::vector<int> dailyMinutes;
    int totalMinutes = 0;

    for (const auto& day : schedule.week) {
        int dayMinutes = 0;
        for (const auto& item : day.day_items) {
            dayMinutes += timeToMinutes(item.end) - timeToMinutes(item.start);
        }
        dailyMinutes.push_back(dayMinutes);
        totalMinutes += dayMinutes;
    }

    if (dailyMinutes.empty()) return 0.0;

    double mean = static_cast<double>(totalMinutes) / dailyMinutes.size();
    double variance = 0.0;

    for (int minutes : dailyMinutes) {
        variance += (minutes - mean) * (minutes - mean);
    }

    return variance / dailyMinutes.size();
}

// Helper function to calculate blocked time (total time from earliest to latest session)
int calculateBlockedTime(const InformativeSchedule& schedule) {
    int earliestOverall = 1440;
    int latestOverall = 0;
    bool hasAnySession = false;

    for (const auto& day : schedule.week) {
        if (!day.day_items.empty()) {
            hasAnySession = true;
            for (const auto& item : day.day_items) {
                int start = timeToMinutes(item.start);
                int end = timeToMinutes(item.end);
                earliestOverall = std::min(earliestOverall, start);
                latestOverall = std::max(latestOverall, end);
            }
        }
    }

    return hasAnySession ? (latestOverall - earliestOverall) : 0;
}

// Main function to convert a single InformativeSchedule to ScheduleMeta
ScheduleMeta convertToScheduleMeta(const InformativeSchedule& schedule) {
    ScheduleMeta meta;

    // Basic fields from InformativeSchedule
    meta.index = schedule.index;
    meta.amount_days = schedule.amount_days;
    meta.amount_gaps = schedule.amount_gaps;
    meta.gaps_time = schedule.gaps_time;
    meta.avg_start = schedule.avg_start;
    meta.avg_end = schedule.avg_end;

    // Initialize calculated fields
    meta.total_minutes = 0;
    meta.total_sessions = 0;
    meta.max_sessions_per_day = 0;
    meta.days_with_single_session = 0;
    meta.earliest_start = 1440;
    meta.latest_end = 0;
    meta.longest_gap = 0;

    // Process each day
    for (const auto& day : schedule.week) {
        DayMeta dayMeta = calculateDayMeta(day);
        meta.week.push_back(dayMeta);

        meta.total_minutes += dayMeta.total_minutes;
        meta.total_sessions += dayMeta.num_sessions;
        meta.max_sessions_per_day = std::max(meta.max_sessions_per_day, dayMeta.num_sessions);
        meta.longest_gap = std::max(meta.longest_gap, dayMeta.longest_gap);

        // Update overall earliest/latest times
        if (dayMeta.num_sessions > 0) {
            meta.earliest_start = std::min(meta.earliest_start, dayMeta.earliest_start);
            meta.latest_end = std::max(meta.latest_end, dayMeta.latest_end);
        }

        if (dayMeta.num_sessions == 1) {
            meta.days_with_single_session++;
        }
    }

    // Reset earliest_start if no sessions
    if (meta.total_sessions == 0) {
        meta.earliest_start = 0;
    }

    // Calculate courses metadata
    meta.courses = calculateCourseMeta(schedule);

    // Calculate blocked time
    meta.blocked_time_minutes = calculateBlockedTime(schedule);

    // Calculate new metrics
    meta.workload_variance = calculateWorkloadVariance(schedule);
    meta.free_days = 0;
    for (const auto& dayMeta : meta.week) {
        if (dayMeta.num_sessions == 0) {
            meta.free_days++;
        }
    }

    // Boolean traits
    meta.has_long_gap = schedule.gaps_time > 120; // More than 2 hours of gaps
    meta.has_morning_session = meta.earliest_start < 540; // Before 9:00 AM
    meta.has_late_session = meta.latest_end > 1080; // After 6:00 PM
    meta.has_midweek_break = hasMidweekBreak(schedule);
    meta.has_back_to_back = hasBackToBackSessions(schedule);
    meta.is_balanced = meta.workload_variance < 10000; // Low variance in daily workload
    meta.is_compact = meta.blocked_time_minutes > 0 &&
                      (static_cast<double>(meta.total_minutes) / meta.blocked_time_minutes) > 0.6;

    return meta;
}

// Helper function to convert ScheduleMeta to compact JSON string
std::string scheduleMetaToJson(const ScheduleMeta& meta) {
    std::ostringstream json;
    json << "{\"index\":" << meta.index + 1
         << ",\"amount_days\":" << meta.amount_days
         << ",\"amount_gaps\":" << meta.amount_gaps
         << ",\"gaps_time\":" << meta.gaps_time
         << ",\"avg_start\":" << meta.avg_start
         << ",\"avg_end\":" << meta.avg_end
         << ",\"total_minutes\":" << meta.total_minutes
         << ",\"total_sessions\":" << meta.total_sessions
         << ",\"max_sessions_per_day\":" << meta.max_sessions_per_day
         << ",\"days_with_single_session\":" << meta.days_with_single_session
         << ",\"earliest_start\":" << meta.earliest_start
         << ",\"latest_end\":" << meta.latest_end
         << ",\"longest_gap\":" << meta.longest_gap
         << ",\"blocked_time_minutes\":" << meta.blocked_time_minutes
         << ",\"workload_variance\":" << std::fixed << std::setprecision(2) << meta.workload_variance
         << ",\"free_days\":" << meta.free_days
         << ",\"has_long_gap\":" << (meta.has_long_gap ? "true" : "false")
         << ",\"has_morning_session\":" << (meta.has_morning_session ? "true" : "false")
         << ",\"has_late_session\":" << (meta.has_late_session ? "true" : "false")
         << ",\"has_midweek_break\":" << (meta.has_midweek_break ? "true" : "false")
         << ",\"has_back_to_back\":" << (meta.has_back_to_back ? "true" : "false")
         << ",\"is_balanced\":" << (meta.is_balanced ? "true" : "false")
         << ",\"is_compact\":" << (meta.is_compact ? "true" : "false")
         << ",\"week\":[";

    for (size_t i = 0; i < meta.week.size(); ++i) {
        const auto& day = meta.week[i];
        json << "{\"day\":\"" << escapeJson(day.day) << "\""
             << ",\"num_sessions\":" << day.num_sessions
             << ",\"total_minutes\":" << day.total_minutes
             << ",\"earliest_start\":" << day.earliest_start
             << ",\"latest_end\":" << day.latest_end
             << ",\"longest_gap\":" << day.longest_gap << "}";
        if (i < meta.week.size() - 1) json << ",";
    }

    json << "],\"courses\":[";

    for (size_t i = 0; i < meta.courses.size(); ++i) {
        const auto& course = meta.courses[i];
        json << "{\"raw_id\":\"" << escapeJson(course.raw_id) << "\""
             << ",\"name\":\"" << escapeJson(course.name) << "\""
             << ",\"lectures\":" << course.lectures
             << ",\"tutorials\":" << course.tutorials
             << ",\"labs\":" << course.labs
             << ",\"earliest_start\":" << course.earliest_start
             << ",\"latest_end\":" << course.latest_end << "}";
        if (i < meta.courses.size() - 1) json << ",";
    }

    json << "]}";
    return json.str();
}

// Main function: CalculateMetaData
std::string CalculateMetaData(const std::vector<InformativeSchedule>& schedules) {
    std::ostringstream result;
    result << "[";

    for (size_t i = 0; i < schedules.size(); ++i) {
        ScheduleMeta meta = convertToScheduleMeta(schedules[i]);
        result << scheduleMetaToJson(meta);

        if (i < schedules.size() - 1) {
            result << ",";
        }
    }

    result << "]";
    return result.str();
}