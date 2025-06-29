#include "ScheduleBuilder.h"

using namespace std;

unordered_map<int, CourseInfo> ScheduleBuilder::courseInfoMap;

// Checks if there is a time conflict between two CourseSelections
bool ScheduleBuilder::hasConflict(const CourseSelection& a, const CourseSelection& b) {
    vector<const Session*> aSessions = getSessions(a);
    vector<const Session*> bSessions = getSessions(b);

    // Compare each session in a with each session in b
    for (const auto* s1 : aSessions) {
        for (const auto* s2 : bSessions) {
            if (TimeUtils::isOverlap(s1, s2)) return true;
        }
    }
    return false;
}

// Recursive backtracking function to build all valid schedules
void ScheduleBuilder::backtrack(int currentCourse,
                                const vector<vector<CourseSelection>>& allOptions,
                                vector<CourseSelection>& currentCombination,
                                vector<InformativeSchedule>& results) {
    try {
        if (currentCourse == allOptions.size()) {
            results.push_back(convertToInformativeSchedule(currentCombination, results.size()));
            return;
        }

        for (const auto& option : allOptions[currentCourse]) {
            bool conflict = false;

            for (const auto& selected : currentCombination) {
                if (hasConflict(option, selected)) {
                    conflict = true;
                    break;
                }
            }

            if (!conflict) {
                currentCombination.push_back(option);
                backtrack(currentCourse + 1, allOptions, currentCombination, results);
                currentCombination.pop_back();
            }
        }
    } catch (const exception& e) {
        Logger::get().logError("Exception in ScheduleBuilder::backtrack: " + string(e.what()));
    }
}

// Public method to build all possible valid schedules from a list of courses
vector<InformativeSchedule> ScheduleBuilder::build(const vector<Course>& courses) {
    Logger::get().logInfo("Starting schedule generation for " + to_string(courses.size()) + " courses.");

    vector<InformativeSchedule> results;
    try {
        buildCourseInfoMap(courses);

        CourseLegalComb generator;
        vector<vector<CourseSelection>> allOptions;

        // Generate combinations for each course
        for (const auto& course : courses) {
            auto combinations = generator.generate(course);
            Logger::get().logInfo("Generated " + to_string(combinations.size()) + " combinations for course ID " + to_string(course.id));
            allOptions.push_back(std::move(combinations)); // Store the combinations
        }

        vector<CourseSelection> current;
        backtrack(0, allOptions, current, results);

        Logger::get().logInfo("Finished schedule generation. Total valid schedules: " + to_string(results.size()));
    } catch (const exception& e) {
        // Log any exceptions that occur during schedule generation
        Logger::get().logError("Exception in ScheduleBuilder::build: " + string(e.what()));
    }

    return results;
}

// Helper method to build course info map
void ScheduleBuilder::buildCourseInfoMap(const vector<Course>& courses) {
    courseInfoMap.clear();
    for (const auto& course : courses) {
        courseInfoMap[course.id] = {course.raw_id, course.name};
    }
}

// Converts a vector of CourseSelections to an InformativeSchedule
InformativeSchedule ScheduleBuilder::convertToInformativeSchedule(const vector<CourseSelection>& selections, int index) const {
    InformativeSchedule schedule;
    schedule.index = index;

    try {
        map<int, vector<ScheduleItem>> daySchedules;
        const vector<string> dayNames = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

        for (const auto& selection : selections) {
            // Process lecture group
            if (selection.lectureGroup) {
                processGroupSessions(selection, selection.lectureGroup, "Lecture", daySchedules);
            }

            // Process tutorial group
            if (selection.tutorialGroup) {
                processGroupSessions(selection, selection.tutorialGroup, "Tutorial", daySchedules);
            }

            // Process lab group
            if (selection.labGroup) {
                processGroupSessions(selection, selection.labGroup, "Lab", daySchedules);
            }

            // Process block group
            if (selection.blockGroup) {
                processGroupSessions(selection, selection.blockGroup, "Block", daySchedules);
            }

            // Process departmental sessions group
            if (selection.departmentalGroup) {
                processGroupSessions(selection, selection.departmentalGroup, "Departmental", daySchedules);
            }

            // Process reinforcement group
            if (selection.reinforcementGroup) {
                processGroupSessions(selection, selection.reinforcementGroup, "Reinforcement", daySchedules);
            }

            // Process guidance group
            if (selection.guidanceGroup) {
                processGroupSessions(selection, selection.guidanceGroup, "Guidance", daySchedules);
            }

            // Process colloquium group
            if (selection.colloquiumGroup) {
                processGroupSessions(selection, selection.colloquiumGroup, "Colloquium", daySchedules);
            }

            // Process registration group
            if (selection.registrationGroup) {
                processGroupSessions(selection, selection.registrationGroup, "Registration", daySchedules);
            }

            // Process thesis group
            if (selection.thesisGroup) {
                processGroupSessions(selection, selection.thesisGroup, "Thesis", daySchedules);
            }

            // Process project group
            if (selection.projectGroup) {
                processGroupSessions(selection, selection.projectGroup, "Project", daySchedules);
            }
        }

        for (int day = 0; day < 7; day++) {
            ScheduleDay scheduleDay;
            scheduleDay.day = dayNames[day];

            int algorithmDay = day + 1;

            if (daySchedules.find(algorithmDay) != daySchedules.end()) {
                auto& dayItems = daySchedules[algorithmDay];
                sort(dayItems.begin(), dayItems.end(), [](const ScheduleItem& a, const ScheduleItem& b) {
                    return TimeUtils::toMinutes(a.start) < TimeUtils::toMinutes(b.start);
                });
                scheduleDay.day_items = dayItems;
            }

            schedule.week.push_back(scheduleDay);
        }

        calculateScheduleMetrics(schedule);

    } catch (const exception& e) {
        Logger::get().logError("Exception in convertToInformativeSchedule: " + string(e.what()));
        schedule.week.clear();
        const vector<string> dayNames = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        for (int day = 0; day < 7; day++) {
            ScheduleDay scheduleDay;
            scheduleDay.day = dayNames[day];
            schedule.week.push_back(scheduleDay);
        }
    }

    return schedule;
}

// Helper method to process all sessions in a group and add them to the day schedules
void ScheduleBuilder::processGroupSessions(const CourseSelection& selection,
                                           const Group* group,
                                           const string& sessionType,
                                           map<int, vector<ScheduleItem>>& daySchedules) {
    if (!group) return;

    try {
        string courseName = getCourseNameById(selection.courseId);
        string courseRawId = getCourseRawIdById(selection.courseId);

        for (const auto& session : group->sessions) {

            ScheduleItem item;
            item.courseName = courseName;
            item.raw_id = courseRawId;
            item.type = sessionType;
            item.start = session.start_time;
            item.end = session.end_time;
            item.building = session.building_number;
            item.room = session.room_number;

            daySchedules[session.day_of_week].push_back(item);
        }

    } catch (const exception& e) {
        Logger::get().logError("Exception in processGroupSessions: " + string(e.what()));
    }
}

string ScheduleBuilder::getCourseNameById(int courseId) {
    auto it = courseInfoMap.find(courseId);
    if (it != courseInfoMap.end()) {
        return it->second.name;
    }
    Logger::get().logWarning("Course ID " + to_string(courseId) + " not found in course info map");
    return "Unknown Course";
}

string ScheduleBuilder::getCourseRawIdById(int courseId) {
    auto it = courseInfoMap.find(courseId);
    if (it != courseInfoMap.end()) {
        return it->second.raw_id;
    }
    Logger::get().logWarning("Course ID " + to_string(courseId) + " not found in course info map");
    return "UNKNOWN";
}

void ScheduleBuilder::calculateScheduleMetrics(InformativeSchedule& schedule) {
    int totalDaysWithItems = 0;
    int totalGaps = 0;
    int totalGapTime = 0;
    int totalStartTime = 0;
    int totalEndTime = 0;

    try {
        for (const auto& scheduleDay : schedule.week) {
            if (scheduleDay.day_items.empty()) {
                continue;
            }

            // calculate amount of none empty days
            totalDaysWithItems++;

            // start and end times for day
            int dayStartMinutes = TimeUtils::toMinutes(scheduleDay.day_items.front().start);
            int dayEndMinutes = TimeUtils::toMinutes(scheduleDay.day_items.back().end);

            totalStartTime += dayStartMinutes;
            totalEndTime += dayEndMinutes;

            // Calculate gaps for day
            for (size_t i = 0; i < scheduleDay.day_items.size() - 1; i++) {
                int currentEndMinutes = TimeUtils::toMinutes(scheduleDay.day_items[i].end);
                int nextStartMinutes = TimeUtils::toMinutes(scheduleDay.day_items[i + 1].start);

                int gapDuration = nextStartMinutes - currentEndMinutes;

                if (gapDuration >= 30) {
                    totalGaps++;
                    totalGapTime += gapDuration;
                }
            }
        }

        schedule.amount_days = totalDaysWithItems;
        schedule.amount_gaps = totalGaps;
        schedule.gaps_time = totalGapTime;

        // Calculate averages
        if (totalDaysWithItems > 0) {
            schedule.avg_start = totalStartTime / totalDaysWithItems;
            schedule.avg_end = totalEndTime / totalDaysWithItems;
        } else {
            schedule.avg_start = 0;
            schedule.avg_end = 0;
        }

    } catch (const exception& e) {
        Logger::get().logError("Exception in calculateScheduleMetrics: " + string(e.what()));
        schedule.amount_days = 0;
        schedule.amount_gaps = 0;
        schedule.gaps_time = 0;
        schedule.avg_start = 0;
        schedule.avg_end = 0;
    }
}