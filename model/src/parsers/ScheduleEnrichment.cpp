#include "parsers/ScheduleEnrichment.h"
#include "logs/logger.h"
#include <QDebug>
#include <iostream>

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
        for (int day = 0; day < 7; ++day) {
            if (dayMap.count(day)) {
                ScheduleDay scheduleDay;

                // Create a non-const copy of the vector for sorting
                vector<ScheduleItem> items = dayMap.at(day);

                // Sort the items by start time
                sort(items.begin(), items.end(), [](const ScheduleItem& a, const ScheduleItem& b) {
                    return a.start < b.start;
                });

                // Add the sorted items to the scheduleDay
                scheduleDay.day = items;

                // Add this day to the week
                informativeSchedule.week.push_back(scheduleDay);
            }
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

void printInformativeSchedules(const vector<InformativeSchedule>& schedules) {
    std::cout << "Function was called with " << schedules.size() << " schedules" << std::endl;

    for (InformativeSchedule sched : schedules) {
        qDebug() << "This is a debug message";
        std::cout << "Schedule #" << sched.index << std::endl << std::flush;
        printf("Schedule #%d\n", sched.index);
    }
}