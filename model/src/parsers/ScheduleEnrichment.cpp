#include "parsers/ScheduleEnrichment.h"
#include "logger/logger.h"
#include <algorithm>

using namespace std;

// Helper function to add all sessions from a group to the day map
void addGroupToDayMap(unordered_map<int, vector<ScheduleItem>>& dayMap, const Group* group,
                      const string& sessionType, const CourseInfo& courseInfo) {
    if (!group) return;  // Skip if group is null

    // Iterate through all sessions in the group
    for (const auto& session : group->sessions) {
        ScheduleItem item;
        item.courseName = courseInfo.name;
        item.raw_id = courseInfo.raw_id;
        item.type = sessionType;
        item.start = session.start_time;
        item.end = session.end_time;
        item.building = session.building_number;
        item.room = session.room_number;

        dayMap[session.day_of_week].push_back(item);
    }
}

// Build a day-indexed map of schedule items
unordered_map<int, vector<ScheduleItem>> buildDayMap(const Schedule& schedule,
                                                     const unordered_map<int, CourseInfo>& courseInfoMap) {
    unordered_map<int, vector<ScheduleItem>> dayMap;

    for (const auto& cs : schedule.selections) {
        auto courseInfoIt = courseInfoMap.find(cs.courseId);
        if (courseInfoIt == courseInfoMap.end()) {
            Logger::get().logWarning("Course info not found for course ID: " + to_string(cs.courseId));
            continue;
        }

        const CourseInfo& courseInfo = courseInfoIt->second;

        // Use the new function that handles Groups instead of individual Sessions
        addGroupToDayMap(dayMap, cs.lecture, "lecture", courseInfo);
        addGroupToDayMap(dayMap, cs.tutorial, "tutorial", courseInfo);
        addGroupToDayMap(dayMap, cs.lab, "lab", courseInfo);
    }

    // Sort sessions by start time for each day
    for (auto& [day, sessions] : dayMap) {
        sort(sessions.begin(), sessions.end(), [](const ScheduleItem& a, const ScheduleItem& b) {
            return a.start < b.start;
        });
    }

    return dayMap;
}

// Implementation of the missing exportSchedulesToObjects function
vector<InformativeSchedule> exportSchedulesToObjects(const vector<Schedule>& schedules,
                                                     const vector<Course>& courses) {
    vector<InformativeSchedule> informativeSchedules;

    // Build course info map for quick lookup
    unordered_map<int, CourseInfo> courseInfoMap = buildCourseInfoMap(courses);

    // Convert each Schedule to InformativeSchedule
    for (size_t i = 0; i < schedules.size(); ++i) {
        InformativeSchedule infoSchedule = createInformativeSchedule(schedules[i], i, courseInfoMap);
        informativeSchedules.push_back(infoSchedule);
    }

    return informativeSchedules;
}

// Implementation of buildCourseInfoMap function
unordered_map<int, CourseInfo> buildCourseInfoMap(const vector<Course>& courses) {
    unordered_map<int, CourseInfo> courseInfoMap;

    for (const auto& course : courses) {
        CourseInfo info;
        info.raw_id = course.raw_id;
        info.name = course.name;
        courseInfoMap[course.id] = info;
    }

    return courseInfoMap;
}

// Implementation of createInformativeSchedule function
InformativeSchedule createInformativeSchedule(const Schedule& schedule, size_t index,
                                              const unordered_map<int, CourseInfo>& courseInfoMap) {
    InformativeSchedule infoSchedule;
    infoSchedule.index = static_cast<int>(index);

    // Build the day map
    auto dayMap = buildDayMap(schedule, courseInfoMap);

    // Convert dayMap to vector of ScheduleDay objects
    for (int day = 1; day <= 7; ++day) {
        if (dayMap.find(day) != dayMap.end() && !dayMap[day].empty()) {
            ScheduleDay scheduleDay;
            scheduleDay.day = dayToString(day);
            scheduleDay.day_items = dayMap[day];
            infoSchedule.week.push_back(scheduleDay);
        }
    }

    return infoSchedule;
}

// Implementation of dayToString function
string dayToString(int day) {
    switch (day) {
        case 1: return "Sunday";
        case 2: return "Monday";
        case 3: return "Tuesday";
        case 4: return "Wednesday";
        case 5: return "Thursday";
        case 6: return "Friday";
        case 7: return "Saturday";
        default: return "Unknown";
    }
}

// You'll also need to implement other declared functions like:
// - isScheduleValid
// - createScheduleDay
// - dayToString
// - printInformativeSchedules
//
// These implementations depend on your specific ScheduleItem and ScheduleDay structures