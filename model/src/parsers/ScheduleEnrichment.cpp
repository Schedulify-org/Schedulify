#include "parsers/ScheduleEnrichment.h"

unordered_map<int, CourseInfo> buildCourseInfoMap(const vector<Course>& courses) {
    unordered_map<int, CourseInfo> courseInfoMap;
    courseInfoMap.reserve(courses.size());

    for (const auto& course : courses) {
        courseInfoMap[course.id] = {course.raw_id, course.name};
    }
    return courseInfoMap;
}


void addSessionToDayMap(unordered_map<int, vector<ScheduleItem>>& dayMap, const Session* session,
                        const string& sessionType, const CourseInfo& courseInfo) {
    if (session) {
        dayMap[session->day_of_week].push_back({courseInfo.name, courseInfo.raw_id,sessionType,
                                                session->start_time,session->end_time,
                                                session->building_number,session->room_number});
    }
}


unordered_map<int, vector<ScheduleItem>> buildDayMap(const Schedule& schedule,
        const unordered_map<int, CourseInfo>& courseInfoMap) {

    unordered_map<int, vector<ScheduleItem>> dayMap;
    for (int day = 1; day <= 7; ++day) {
        dayMap[day].reserve(schedule.selections.size() * 2);
    }

    for (const CourseSelection& cs : schedule.selections) {
        const auto& courseInfo = courseInfoMap.count(cs.courseId) ?
                                 courseInfoMap.at(cs.courseId) : CourseInfo{to_string(cs.courseId), to_string(cs.courseId)};

        addSessionToDayMap(dayMap, cs.lecture, "lecture", courseInfo);
        addSessionToDayMap(dayMap, cs.tutorial, "tutorial", courseInfo);
        addSessionToDayMap(dayMap, cs.lab, "lab", courseInfo);
    }

    return dayMap;
}


bool isScheduleValid(const unordered_map<int, vector<ScheduleItem>>& dayMap) {
    for (const auto& [day, items] : dayMap) {
        if (!items.empty()) {
            return true;
        }
    }
    return false;
}


ScheduleDay createScheduleDay(int day, unordered_map<int, vector<ScheduleItem>>& dayMap) {
    ScheduleDay scheduleDay;
    scheduleDay.day = dayToString(day);

    if (dayMap.count(day) && !dayMap[day].empty()) {
        // Sort items by start time
        auto& items = dayMap[day];
        sort(items.begin(), items.end(), [](const ScheduleItem& a, const ScheduleItem& b) {
            return a.start < b.start;
        });

        scheduleDay.day_items = std::move(items);
    }

    return scheduleDay;
}


InformativeSchedule createInformativeSchedule(const Schedule& schedule, size_t index,
        const unordered_map<int, CourseInfo>& courseInfoMap) {

    auto dayMap = buildDayMap(schedule, courseInfoMap);

    if (!isScheduleValid(dayMap)) {
        return {}; // Return empty to indicate invalid
    }

    InformativeSchedule informativeSchedule;
    informativeSchedule.index = index;
    informativeSchedule.week.reserve(7);

    for (int day = 1; day <= 7; ++day) {
        informativeSchedule.week.push_back(createScheduleDay(day, dayMap));
    }

    return informativeSchedule;
}

vector<InformativeSchedule> exportSchedulesToObjects(const vector<Schedule>& schedules, const vector<Course>& courses) {
    Logger::get().logInfo("Initiate schedule enrichment");

    auto courseInfoMap = buildCourseInfoMap(courses);

    vector<InformativeSchedule> result;
    size_t validCount = 0;

    for (size_t i = 0; i < schedules.size(); ++i) {
        InformativeSchedule schedule = createInformativeSchedule(schedules[i], i + 1, courseInfoMap);

        if (schedule.week.empty()) {
            continue; // Skip invalid schedules
        }

        validCount++;
        result.push_back(std::move(schedule));
    }

    if (validCount == 0) {
        Logger::get().logError("There are no valid schedules, aborting...");
        return {};
    }
    Logger::get().logInfo("Done working on " + to_string(validCount) + " valid schedules");

    return result;
}


string dayToString(const int day) {
    static const string days[] = {
            "sunday", "monday", "tuesday", "wednesday",
            "thursday", "friday", "saturday"
    };
    return (day >= 1 && day <= 7) ? days[day - 1] : "unknown";
}


void printInformativeSchedules(const vector<InformativeSchedule>& schedules) {
    for (const auto& [index, week] : schedules) {
        cout << "----------------------" << endl;
        cout << "Schedule: " + to_string(index) << endl;
        for (const auto& [day, day_items] : week) {
            cout << day << endl;
            for (const auto& [courseName, raw_id, type, start, end, building, room] : day_items) {
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