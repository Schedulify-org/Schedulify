#ifndef SCHEDULE_ENRICHMENT_H
#define SCHEDULE_ENRICHMENT_H

#include "model_interfaces.h"
#include "main/inner_structs.h"
#include "logger/logger.h"

#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>

using std::string;
using std::cout;
using std::vector;

// Converts schedules to InformativeSchedule objects.
std::vector<InformativeSchedule> exportSchedulesToObjects(const vector<Schedule>& schedules,
        const vector<Course>& courses);

// Creates a mapping from course IDs to CourseInfo objects.
std::unordered_map<int, CourseInfo> buildCourseInfoMap(const vector<Course>& courses);

// Adds a session to the day map if it exists.
void addSessionToDayMap(std::unordered_map<int, vector<ScheduleItem>>& dayMap, const Session* session,
        const string& sessionType,
        const CourseInfo& courseInfo);

// Builds a day map for a schedule.
std::unordered_map<int, std::vector<ScheduleItem>> buildDayMap(const Schedule& schedule,
        const std::unordered_map<int, CourseInfo>& courseInfoMap);


// checks if a schedule is valid.
bool isScheduleValid(const std::unordered_map<int, vector<ScheduleItem>>& dayMap);

// Creates a ScheduleDay object for a specific day.
ScheduleDay createScheduleDay(int day, std::unordered_map<int, vector<ScheduleItem>>& dayMap);

// Creates an InformativeSchedule from a Schedule.
InformativeSchedule createInformativeSchedule(const Schedule& schedule, size_t index,
        const std::unordered_map<int, CourseInfo>& courseInfoMap);

// Processes a range of schedules in a thread.
void processScheduleRange(
        const std::vector<Schedule>& schedules,
        const std::unordered_map<int, CourseInfo>& courseInfoMap,
        size_t startIdx,
        size_t endIdx,
        size_t threadIdx,
        std::vector<std::vector<InformativeSchedule>>& threadResults,
        std::atomic<size_t>& validCount);

// Calculates the optimal thread count based on data size.
unsigned int calculateOptimalThreadCount(size_t dataSize);

// Launches processing threads.
std::vector<std::thread> launchProcessingThreads(const vector<Schedule>& schedules,
        const std::unordered_map<int, CourseInfo>& courseInfoMap,
        unsigned int numThreads,
        vector<std::vector<InformativeSchedule>>& threadResults,
        std::atomic<size_t>& validCount);


// Combines results from all threads.
std::vector<InformativeSchedule> combineThreadResults( vector<vector<InformativeSchedule>>& threadResults,
        size_t validCount);

// Converts a day number to a string representation.
std::string dayToString(int day);

// Prints informative schedules to the console.
void printInformativeSchedules(const vector<InformativeSchedule>& schedules);


#endif //SCHEDULE_ENRICHMENT_H
