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

        // Move items instead of copying
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

    vector<InformativeSchedule> results[2];
    results[0].reserve(schedules.size() / 4);
    results[1].reserve(schedules.size() / 4);

    size_t half = schedules.size() / 2;

    // Create and launch two threads
    thread thread1([&]() {
        for (size_t i = 0; i < half; ++i) {
            InformativeSchedule schedule = createInformativeSchedule(schedules[i], i + 1, courseInfoMap);
            if (!schedule.week.empty()) {
                results[0].push_back(std::move(schedule));
            }
        }
    });

    thread thread2([&]() {
        for (size_t i = half; i < schedules.size(); ++i) {
            InformativeSchedule schedule = createInformativeSchedule(schedules[i], i + 1, courseInfoMap);
            if (!schedule.week.empty()) {
                results[1].push_back(std::move(schedule));
            }
        }
    });

    thread1.join();
    thread2.join();

    size_t validCount = results[0].size() + results[1].size();
    vector<InformativeSchedule> finalResult;
    finalResult.reserve(validCount);

    size_t currentIndex = 1;
    for (auto& schedule : results[0]) {
        schedule.index = currentIndex++;
        finalResult.push_back(std::move(schedule));
    }

    for (auto& schedule : results[1]) {
        schedule.index = currentIndex++;
        finalResult.push_back(std::move(schedule));
    }

    if (validCount == 0) {
        Logger::get().logError("There are no valid schedules, aborting...");
        return {};
    }

    Logger::get().logInfo("Done working on " + to_string(validCount) + " valid schedules");

    return finalResult;
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