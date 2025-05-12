#include "parsers/ScheduleEnrichment.h"
#include "logger/logger.h"

vector<InformativeSchedule> exportSchedulesToObjects(const vector<Schedule>& schedules, const vector<Course>& courses) {
    vector<InformativeSchedule> result;

    vector<Schedule> validSchedules = filterValidSchedules(schedules, courses);

    if (validSchedules.empty()) {
        Logger::get().logError("There are no valid schedules, aborting...");
        return result;
    }

    // Create InformativeSchedule objects for each valid schedule
    for (size_t i = 0; i < validSchedules.size(); ++i) {
        InformativeSchedule informativeSchedule;
        informativeSchedule.index = i + 1;

        const auto& schedule = validSchedules[i];

        // Build the dayMap for the current schedule
        auto dayMap = buildDayMapForSchedule(schedule, courses, exportType::VECTOR);

        // Create ScheduleDay objects for each day
        for (int day = 1; day <= 7; ++day) {
            ScheduleDay scheduleDay;
            scheduleDay.day = dayToString(day);

            if (dayMap.count(day)) {
                // Create a non-const copy of the vector for sorting
                vector<ScheduleItem> items = dayMap.at(day);

                // Sort the items by start time
                sort(items.begin(), items.end(), [](const ScheduleItem& a, const ScheduleItem& b) {
                    return a.start < b.start;
                });

                // Add the sorted items to the scheduleDay
                scheduleDay.day_items = items;
            } else
            {
                scheduleDay.day_items = {};
            }
            // Add this day to the week
            informativeSchedule.week.push_back(scheduleDay);
        }

        // Add this schedule to the result
        result.push_back(informativeSchedule);
    }

    return result;
}

vector<Schedule> filterValidSchedules(const vector<Schedule>& schedules, const vector<Course>& courses) {
    vector<Schedule> validSchedules;

    for (const auto& schedule : schedules) {
        auto dayMap = buildDayMapForSchedule(schedule, courses, exportType::VECTOR);

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

    return validSchedules;
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

CourseInfo getCourseInfoById(const vector<Course>& courses, int courseId, exportType type) {
    auto it = find_if(courses.begin(), courses.end(), [&](const Course& c) {
        return c.id == courseId;
    });
    if (it != courses.end()) {
        string name;
        switch (type) {
            case exportType::JSON:
                name = "\"" + it->name + "\"";
                break;
            case exportType::VECTOR:
                name = it->name;
                break;
        }
        return {it->raw_id, name};
    } else {
        string fallback = to_string(courseId);
        return {fallback, fallback};
    }
}

unordered_map<int, vector<ScheduleItem>> buildDayMapForSchedule(const Schedule& schedule,
                                                                const vector<Course>& courses, exportType type) {
    unordered_map<int, vector<ScheduleItem>> dayMap;

    for (const CourseSelection& cs : schedule.selections) {
        CourseInfo info = getCourseInfoById(courses, cs.courseId, type);

        addSessionToDayMap(dayMap, cs.lecture, "lecture", info.name, info.raw_id);
        addSessionToDayMap(dayMap, cs.tutorial, "tutorial", info.name, info.raw_id);
        addSessionToDayMap(dayMap, cs.lab, "lab", info.name, info.raw_id);
    }

    return dayMap;
}

string dayToString(const int day) {
    static const string days[] = {
        "sunday", "monday", "tuesday", "wednesday",
        "thursday", "friday", "saturday"
    };
    return (day >= 1 && day <= 7) ? days[day - 1] : "unknown";
}

void printInformativeSchedules(const vector<InformativeSchedule>& schedules) {
    for (const auto& [index, week] : schedules)
        {
        cout << "----------------------" << endl;
        cout << "Schedule: " + to_string(index) << endl;
        for (const auto& [day, day_items] : week)
            {
            cout << day << endl;
            for (const auto& [courseName, raw_id, type, start, end, building, room] : day_items)
            {
                cout << "   courseName: " + courseName << endl;
                cout << "   raw_id: " + raw_id << endl;
                cout << "   type: " + type << endl;
                cout << "   start: " + start << endl;
                cout << "   end: " + end << endl;
                cout << "   building: " + building << endl;
                cout << "   room: " + room << endl;
                cout << "" << endl;
            }
        }
    }
}