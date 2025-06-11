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

// Helper function to calculate DayMeta for a given day
DayMeta calculateDayMeta(const ScheduleDay& day) {
    DayMeta dayMeta;
    dayMeta.day = day.day;
    dayMeta.num_sessions = day.day_items.size();
    dayMeta.total_minutes = 0;
    dayMeta.earliest_start = 1440;
    dayMeta.latest_end = 0;

    for (const auto& item : day.day_items) {
        int start = timeToMinutes(item.start);
        int end = timeToMinutes(item.end);

        dayMeta.total_minutes += (end - start);
        dayMeta.earliest_start = std::min(dayMeta.earliest_start, start);
        dayMeta.latest_end = std::max(dayMeta.latest_end, end);
    }

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

    // Calculate additional fields
    meta.total_minutes = 0;
    meta.total_sessions = 0;
    meta.max_sessions_per_day = 0;
    meta.days_with_single_session = 0;

    // Process each day
    for (const auto& day : schedule.week) {
        DayMeta dayMeta = calculateDayMeta(day);
        meta.week.push_back(dayMeta);

        meta.total_minutes += dayMeta.total_minutes;
        meta.total_sessions += dayMeta.num_sessions;
        meta.max_sessions_per_day = std::max(meta.max_sessions_per_day, dayMeta.num_sessions);

        if (dayMeta.num_sessions == 1) {
            meta.days_with_single_session++;
        }
    }

    // Calculate courses metadata
    meta.courses = calculateCourseMeta(schedule);

    // Calculate blocked time
    meta.blocked_time_minutes = calculateBlockedTime(schedule);

    // Boolean traits
    meta.has_long_gap = schedule.gaps_time > 120; // More than 2 hours of gaps
    meta.has_morning_session = schedule.avg_start < 540; // Before 9:00 AM
    meta.has_late_session = schedule.avg_end > 1080; // After 6:00 PM
    meta.has_midweek_break = hasMidweekBreak(schedule);

    return meta;
}

// Helper function to convert ScheduleMeta to compact JSON string
std::string scheduleMetaToJson(const ScheduleMeta& meta) {
    std::ostringstream json;
    json << "{\"index\":" << meta.index
         << ",\"amount_days\":" << meta.amount_days
         << ",\"amount_gaps\":" << meta.amount_gaps
         << ",\"gaps_time\":" << meta.gaps_time
         << ",\"avg_start\":" << meta.avg_start
         << ",\"avg_end\":" << meta.avg_end
         << ",\"total_minutes\":" << meta.total_minutes
         << ",\"total_sessions\":" << meta.total_sessions
         << ",\"max_sessions_per_day\":" << meta.max_sessions_per_day
         << ",\"days_with_single_session\":" << meta.days_with_single_session
         << ",\"blocked_time_minutes\":" << meta.blocked_time_minutes
         << ",\"has_long_gap\":" << (meta.has_long_gap ? "true" : "false")
         << ",\"has_morning_session\":" << (meta.has_morning_session ? "true" : "false")
         << ",\"has_late_session\":" << (meta.has_late_session ? "true" : "false")
         << ",\"has_midweek_break\":" << (meta.has_midweek_break ? "true" : "false")
         << ",\"week\":[";

    for (size_t i = 0; i < meta.week.size(); ++i) {
        const auto& day = meta.week[i];
        json << "{\"day\":\"" << escapeJson(day.day) << "\""
             << ",\"num_sessions\":" << day.num_sessions
             << ",\"total_minutes\":" << day.total_minutes
             << ",\"earliest_start\":" << day.earliest_start
             << ",\"latest_end\":" << day.latest_end << "}";
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