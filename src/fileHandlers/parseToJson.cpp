#include "fileHandlers//parseToJson.h"
#include "logs/logger.h"

bool exportSchedulesToJson(const vector<Schedule>& schedules, const string& outputPath, const vector<Course>& courses) {
    ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        ostringstream message;
        message << "Cannot open file: " << outputPath << ", aborting...";
        Logger::get().logError(message.str());
        return false;
    }

    vector<Schedule> validSchedules;

    for (const auto& schedule : schedules) {
        auto dayMap = buildDayMapForSchedule(schedule, courses);

        bool hasItems = false;
        for (const auto& [day, items] : dayMap) {
            if (!items.empty()) {
                hasItems = true;
                break;
            }
        }

        if (hasItems) {
            validSchedules.push_back(schedule);
        }
    }

    if (validSchedules.empty()) {
        Logger::get().logError("there are no valid schedules, aborting...");
        return false;
    }

    outFile << "[";

    for (size_t i = 0; i < validSchedules.size(); ++i) {
        const auto& schedule = validSchedules[i];
        outFile << "{\"schedule_number\":" << (i + 1) << ",\"schedule\":[";

        // Build the dayMap for the current schedule
        auto dayMap = buildDayMapForSchedule(schedule, courses);

        bool dayWritten = false;
        for (int day = 0; day < 7; ++day) {
            if (dayMap.count(day)) {
                if (dayWritten) outFile << ",";
                dayWritten = true;

                // Sort schedule items by start time
                vector<ScheduleItem>& items = dayMap[day];
                sort(items.begin(), items.end(), [](const ScheduleItem& a, const ScheduleItem& b) {
                    return a.start < b.start;
                });

                // Writing the schedule for the current day to JSON
                outFile << R"({"day":")" << dayToString(day) << R"(","schedule_items":[)";
                for (size_t j = 0; j < items.size(); ++j) {
                    outFile << ScheduleItemToJson(items[j]);
                    if (j + 1 < items.size()) outFile << ",";
                }
                outFile << "]}";
            }
        }

        outFile << "]}";
        if (i + 1 < schedules.size()) outFile << ",";
    }

    outFile << "]";
    outFile.close();

    return true;
}

string ScheduleItemToJson(const ScheduleItem& s) {
    stringstream ss;
    ss << "{\"course_name\":" << s.courseName
       << R"(,"course_id":")" << s.raw_id << "\""
       << R"(,"item_type":")" << s.type << "\""
       << R"(,"start":")" << s.start << "\""
       << R"(,"end":")" << s.end << "\""
       << R"(,"building":")" << s.building << "\""
       << R"(,"room":")" << s.room << "\"}";
    return ss.str();
}


void addSessionToDayMap(unordered_map<int, vector<ScheduleItem>>& dayMap, const Session* session, const string& type,
        const string& courseName, const string& raw_id) {
    if (!session) return;
    dayMap[session->day_of_week].push_back(ScheduleItem{
            courseName,
            raw_id,
            type,
            session->start_time,
            session->end_time,
            session->building_number,
            session->room_number
    });
}

CourseInfo getCourseInfoById(const vector<Course>& courses, int courseId) {
    auto it = find_if(courses.begin(), courses.end(), [&](const Course& c) {
        return c.id == courseId;
    });
    if (it != courses.end()) {
        return {it->raw_id, "\"" + it->name + "\""};
    } else {
        string fallback = to_string(courseId);
        return {fallback, fallback};
    }
}

unordered_map<int, vector<ScheduleItem>> buildDayMapForSchedule(const Schedule& schedule,
                                                                const vector<Course>& courses) {
    unordered_map<int, vector<ScheduleItem>> dayMap;

    for (const CourseSelection& cs : schedule.selections) {
        CourseInfo info = getCourseInfoById(courses, cs.courseId);

        addSessionToDayMap(dayMap, cs.lecture, "lecture", info.name, info.raw_id);
        addSessionToDayMap(dayMap, cs.tutorial, "tutorial", info.name, info.raw_id);
        addSessionToDayMap(dayMap, cs.lab, "lab", "\"" + info.name + "\"", info.raw_id);
    }

    return dayMap;
}

string dayToString(int day) {
    static const string days[] = {
            "sunday", "monday", "tuesday", "wednesday",
            "thursday", "friday", "saturday"
    };
    return (day >= 1 && day <= 7) ? days[day - 1] : "unknown";
}

