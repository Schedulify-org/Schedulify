// ScheduleBuilder.cpp
#include "../include/ScheduleBuilder.h"
#include "../include/CourseLegalComb.h"
#include "../include/schedule_types.h"
#include "../include/TimeUtils.h"

vector<const Session*> getSessions(const CourseSelection& cs) {
    vector<const Session*> sessions;
    if (cs.lecture) sessions.push_back(cs.lecture);
    if (cs.tutorial) sessions.push_back(cs.tutorial);
    if (cs.lab) sessions.push_back(cs.lab);
    return sessions;
}

// Checks whether there is a time conflict between two CourseSelections
bool ScheduleBuilder::hasConflict(const CourseSelection& a, const CourseSelection& b) const {
    vector<const Session*> aSessions = getSessions(a);
    vector<const Session*> bSessions = getSessions(b);

    for (const auto* s1 : aSessions) {
        for (const auto* s2 : bSessions) {
            if (TimeUtils::isOverlap(s1, s2)) return true;
        }
    }
    return false;
}

// Recursive backtracking function:
// Builds all valid combinations of CourseSelections (one from each course) without conflicts
void ScheduleBuilder::backtrack(int currentCourse, const vector<vector<CourseSelection>>& allOptions, vector<CourseSelection>& currentCombination, vector<Schedule>& results) {

    if (currentCourse == allOptions.size()) {
        results.push_back({currentCombination});
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
        // If no conflicts, add it and continue recursively
        if (!conflict) {
            current.push_back(option);
            backtrack(currentCourse + 1, allOptions, currentCombination, results);
            current.pop_back();
        }
    }
}
// Main entry point for building schedules:
// Generates valid combinations for each course and launches the backtracking search
vector<Schedule> ScheduleBuilder::build(const vector<Course>& courses) {
    CourseLegalComb generator;
    vector<vector<CourseSelection>> allOptions;

    // Generate all valid combinations of sessions for each course
    for (const auto& course : courses) {
        allOptions.push_back(generator.generate(course));
    }

    // Start the recursive backtracking
    vector<Schedule> results;
    vector<CourseSelection> current;
    backtrack(0, allOptions, current, results);
    return results;
}