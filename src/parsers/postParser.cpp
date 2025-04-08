#include "parsers/postParser.h"
#include <iomanip>

void exportCompactJson(const vector<Schedule>& schedules, const string& outputPath) {
    ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        cerr << "Failed to open output file: " << outputPath << endl;
        return;
    }

    outFile << "[";

    for (size_t i = 0; i < schedules.size(); ++i) {
        const auto& schedule = schedules[i];
        outFile << "{\"schedule_number\":" << (i + 1) << ",\"courses\":[";

        for (size_t j = 0; j < schedule.selections.size(); ++j) {
            const auto& cs = schedule.selections[j];
            outFile << "{\"course_id\":" << cs.courseId;

            if (cs.lecture) {
                outFile << ",\"lecture\":{"
                        << "\"day\":" << cs.lecture->day_of_week
                        << ",\"start\":\"" << cs.lecture->start_time << "\""
                        << ",\"end\":\"" << cs.lecture->end_time << "\""
                        << ",\"building\":\"" << cs.lecture->building_number << "\""
                        << ",\"room\":\"" << cs.lecture->room_number << "\"}";
            }

            if (cs.tutorial) {
                outFile << ",\"tutorial\":{"
                        << "\"day\":" << cs.tutorial->day_of_week
                        << ",\"start\":\"" << cs.tutorial->start_time << "\""
                        << ",\"end\":\"" << cs.tutorial->end_time << "\""
                        << ",\"building\":\"" << cs.tutorial->building_number << "\""
                        << ",\"room\":\"" << cs.tutorial->room_number << "\"}";
            }

            if (cs.lab) {
                outFile << ",\"lab\":{"
                        << "\"day\":" << cs.lab->day_of_week
                        << ",\"start\":\"" << cs.lab->start_time << "\""
                        << ",\"end\":\"" << cs.lab->end_time << "\""
                        << ",\"building\":\"" << cs.lab->building_number << "\""
                        << ",\"room\":\"" << cs.lab->room_number << "\"}";
            }

            outFile << "}";

            if (j != schedule.selections.size() - 1)
                outFile << ",";
        }

        outFile << "]}";
        if (i != schedules.size() - 1)
            outFile << ",";
    }

    outFile << "]";
    outFile.close();
}

string dayToString(int day) {
    static const string days[] = {
            "sunday", "monday", "tuesday", "wednesday",
            "thursday", "friday", "saturday"
    };
    return (day >= 0 && day < 7) ? days[day] : "unknown";
}

string ScheduleItem::toJson() const {
    stringstream ss;
    ss << "{\"course_id\":" << courseName
       << ",\"item_type\":\"" << type << "\""
       << ",\"start\":\"" << start << "\""
       << ",\"end\":\"" << end << "\""
       << ",\"building\":\"" << building << "\""
       << ",\"room\":\"" << room << "\"}";
    return ss.str();
}

void exportSchedulesByDayJson(const vector<Schedule>& schedules, const string& outputPath, const vector<Course>& courses) {
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
                auto& items = dayMap[day];
                sort(items.begin(), items.end(), [](const ScheduleItem& a, const ScheduleItem& b) {
                    return a.start < b.start;
                });

                outFile << "{\"day\":\"" << dayToString(day) << "\",\"schedule_items\":[";
                for (size_t j = 0; j < items.size(); ++j) {
                    outFile << items[j].toJson();
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

void exportSchedulesByDayText(const vector<Schedule>& schedules, const string& outputPath, const vector<Course>& courses) {
    ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        cerr << "Failed to open output file: " << outputPath << endl;
        return;
    }

    for (size_t i = 0; i < schedules.size(); ++i) {
        const auto& schedule = schedules[i];
        outFile << "schedule " << (i + 1) << ":\n";

        // Map day index to schedule items
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

        for (int day = 0; day < 7; ++day) {
            if (dayMap.count(day)) {
                outFile << "       " << dayToString(day) << ":\n";

                auto& items = dayMap[day];
                sort(items.begin(), items.end(), [](const ScheduleItem& a, const ScheduleItem& b) {
                    return a.start < b.start;
                });

                for (const auto& item : items) {
                    outFile << "              "
                            << item.courseName << " (" << item.raw_id << "), "
                            << item.type << ", "
                            << item.start << "-" << item.end
                            << " in building " << item.building
                            << " room " << item.room << "\n";
                }
            }
        }

        if (i + 1 < schedules.size())
            outFile << "\n";
    }

    outFile.close();
}

