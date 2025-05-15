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


void processScheduleRange(const vector<Schedule>& schedules, const unordered_map<int, CourseInfo>& courseInfoMap,
        size_t startIdx, size_t endIdx, size_t threadIdx, vector<vector<InformativeSchedule>>& threadResults,
        atomic<size_t>& validCount) {

    size_t localValidCount = 0;
    threadResults[threadIdx].reserve((endIdx - startIdx) / 2);

    for (size_t i = startIdx; i < endIdx; ++i) {
        InformativeSchedule schedule = createInformativeSchedule(schedules[i], i + 1, courseInfoMap);

        if (schedule.week.empty()) {
            continue; // Skip invalid schedules
        }

        localValidCount++;
        threadResults[threadIdx].push_back(std::move(schedule));
    }

    validCount += localValidCount;
}


vector<InformativeSchedule> exportSchedulesToObjects(const vector<Schedule>& schedules, const vector<Course>& courses) {
    // Create course info map
    auto courseInfoMap = buildCourseInfoMap(courses);

    // Determine thread count
    unsigned int numThreads = calculateOptimalThreadCount(schedules.size());

    // Create thread-local results
    vector<vector<InformativeSchedule>> threadResults(numThreads);
    atomic<size_t> validCount{0};

    // Launch threads
    vector<thread> threads = launchProcessingThreads(
            schedules, courseInfoMap, numThreads, threadResults, validCount);

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Check if we have any valid schedules
    if (validCount == 0) {
        Logger::get().logError("There are no valid schedules, aborting...");
        return {};
    }

    // Combine results from all threads
    return combineThreadResults(threadResults, validCount);
}


unsigned int calculateOptimalThreadCount(size_t dataSize) {
    unsigned int numThreads = thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;

    // Limit threads based on data size
    numThreads = min(numThreads, static_cast<unsigned int>(dataSize / 1000 + 1));
    return min(numThreads, 16u);
}


vector<thread> launchProcessingThreads(const vector<Schedule>& schedules,
        const unordered_map<int, CourseInfo>& courseInfoMap, unsigned int numThreads,
        vector<vector<InformativeSchedule>>& threadResults, atomic<size_t>& validCount) {

    vector<thread> threads;
    size_t schedulesPerThread = schedules.size() / numThreads;

    for (size_t threadIdx = 0; threadIdx < numThreads; ++threadIdx) {
        size_t startIdx = threadIdx * schedulesPerThread;
        size_t endIdx = (threadIdx == numThreads - 1) ? schedules.size() : (threadIdx + 1) * schedulesPerThread;

        threads.emplace_back(processScheduleRange,
                             ref(schedules),
                             ref(courseInfoMap),
                             startIdx,
                             endIdx,
                             threadIdx,
                             ref(threadResults),
                             ref(validCount));
    }

    return threads;
}


vector<InformativeSchedule> combineThreadResults(vector<vector<InformativeSchedule>>& threadResults, size_t validCount) {

    vector<InformativeSchedule> result;
    result.reserve(validCount);

    // Merge all thread results and fix indices
    size_t currentIndex = 1;
    for (auto& threadResult : threadResults) {
        for (auto& schedule : threadResult) {
            schedule.index = currentIndex++;
            result.push_back(std::move(schedule));
        }
    }

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