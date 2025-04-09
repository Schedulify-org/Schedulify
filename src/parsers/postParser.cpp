#include "parsers/postParser.h"
#include "logs/logger.h"

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

void exportSchedulesToJson(const vector<Schedule>& schedules, const string& outputPath, const vector<Course>& courses) {
    ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        cerr << "Failed to open output file: " << outputPath << endl;
        return;
    }

    outFile << "[";

    for (size_t i = 0; i < schedules.size(); ++i) {
        const auto& schedule = schedules[i];
        outFile << "{\"schedule_number\":" << (i + 1) << ",\"schedule\":[";

        // Map: day index → vector of schedule items
        unordered_map<int, vector<ScheduleItem>> dayMap;

        for (const auto& cs : schedule.selections) {

            string raw_id;
            string courseName;

            auto it = find_if(courses.begin(), courses.end(), [&](const Course& c) { return c.id == cs.courseId; });
            if (it != courses.end()) {
                raw_id = it->raw_id;
                courseName = it->name;
            } else {
                raw_id = to_string(cs.courseId);
                courseName = to_string(cs.courseId);
            }


            auto add = [&](const Session* s, const string& type) {
                if (!s) return;
                dayMap[s->day_of_week].push_back(ScheduleItem{
                        courseName,
                        raw_id,
                        type,
                        s->start_time,
                        s->end_time,
                        s->building_number,
                        s->room_number
                });
            };
            add(cs.lecture, "lecture");
            add(cs.tutorial, "tutorial");
            add(cs.lab, "lab");
        }

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
}

bool exportSchedulesToText(const vector<Schedule>& schedules, const string& outputPath, const vector<Course>& courses) {
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

    ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        ostringstream message;
        message << "Cannot open file: " << outputPath << ", aborting...";
        Logger::get().logError(message.str());
        return false;
    }

    for (size_t i = 0; i < validSchedules.size(); ++i) {
        writeScheduleToFile(outFile, validSchedules[i], i, courses);
        if (i + 1 < validSchedules.size())
            outFile << "\n";
    }
    outFile.close();

    return true;
}

string dayToString(int day) {
    static const string days[] = {
            "sunday", "monday", "tuesday", "wednesday",
            "thursday", "friday", "saturday"
    };
    return (day >= 1 && day <= 7) ? days[day - 1] : "unknown";
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
        return {it->raw_id, it->name};
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
        addSessionToDayMap(dayMap, cs.lab, "lab", info.name, info.raw_id);
    }

    return dayMap;
}

void writeDayScheduleToFile(ostream& out, int day, const vector<ScheduleItem>& items) {
    out << "       " << dayToString(day) << ":\n";

    vector<ScheduleItem> sortedItems = items;
    sort(sortedItems.begin(), sortedItems.end(), [](const ScheduleItem& a, const ScheduleItem& b) {
        return a.start < b.start;
    });

    for (const ScheduleItem& item : sortedItems) {
        out << "              "
                << item.courseName << " (" << item.raw_id << "), "
                << item.type << ", "
                << item.start << "-" << item.end
                << " in building " << item.building
                << " room " << item.room << "\n";
    }
}

void writeScheduleToFile(ostream& out, const Schedule& schedule, size_t index, const vector<Course>& courses) {
    out << "schedule " << (index + 1) << ":\n";
    auto dayMap = buildDayMapForSchedule(schedule, courses);

    for (int day = 0; day < 7; ++day) {
        if (dayMap.count(day)) {
            writeDayScheduleToFile(out, day, dayMap[day]);
        }
    }
}


