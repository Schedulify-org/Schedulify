#include <vector>
#include <string>
#include "../include/algoritm.h"
#include "schedule_algorithm/getSession.h"

using namespace std;

// Global vector containing all possible schedules (across all courses).
vector<Schedule> allPossibleSchedules;

// Function to convert a time string (HH:MM) into the number of minutes from midnight.
int timeToMinutes(const string& t) {
    int colonPos = t.find(':');
    int hours = stoi(t.substr(0, colonPos));
    int minutes = stoi(t.substr(colonPos + 1));
    return hours * 60 + minutes;
}

// Checks if two sessions conflict (they overlap if on the same day and their times overlap).
bool isOverlap(const Session* s1, const Session* s2) {
    if (s1->day_of_week != s2->day_of_week)
        return false;
    int start1 = timeToMinutes(s1->start_time);
    int end1   = timeToMinutes(s1->end_time);
    int start2 = timeToMinutes(s2->start_time);
    int end2   = timeToMinutes(s2->end_time);
    return (start1 < end2 && start2 < end1);
}

// Returns all valid combinations of sessions for a specific course.
// If the course provides tutorials then a valid combination must include one;
// if it provides labs then a valid combination must include one.
// (Only if a course does not provide tutorials or labs is it acceptable to have a combination with only a lecture.)
vector<CourseSelection> getValidCourseCombinations(int courseId,
    const vector<const Session*>& lectures, 
    const vector<const Session*>& tutorials, 
    const vector<const Session*>& labs) {
    
    vector<CourseSelection> validCombinations;
    for (const auto* lecture : lectures) {
        // If tutorials exist, we must choose one.
        if (!tutorials.empty()) {
            for (const auto* tutorial : tutorials) {
                if (isOverlap(lecture, tutorial))
                    continue;  // Skip if lecture and tutorial overlap.
                // If labs exist, we must choose one.
                if (!labs.empty()) {
                    for (const auto* lab : labs) {
                        if (isOverlap(lecture, lab) || isOverlap(tutorial, lab))
                            continue; // Skip if there is any overlap.
                        validCombinations.push_back({courseId, lecture, tutorial, lab});
                    }
                } else {
                    // No labs exist, so combination must include lecture and tutorial.
                    validCombinations.push_back({courseId, lecture, tutorial, nullptr}); //c
                }
            }
        }
        // If tutorials do NOT exist but labs do, then combination must include a lab.
        else if (!labs.empty()) {
            for (const auto* lab : labs) {
                if (isOverlap(lecture, lab))
                    continue;
                validCombinations.push_back({courseId, lecture, nullptr, lab});
            }
        }
        // If neither tutorials nor labs exist, then only a lecture is available.
        else {
            validCombinations.push_back({courseId, lecture, nullptr, nullptr});
        }
    }
    return validCombinations;
}


// Checks for conflict between the sessions in current selections and a new selection.
bool noConflict(const vector<CourseSelection>& current, const CourseSelection& cs) {
    vector<const Session*> newSessions = getSessions(cs);
    for (const auto& sel : current) {
        vector<const Session*> existingSessions = getSessions(sel);
        for (const auto* s1 : existingSessions) {
            for (const auto* s2 : newSessions) {
                if (isOverlap(s1, s2))
                    return false;
            }
        }
    }
    return true;
}

// Backtracking function: For each course (represented in courseOptions),
// choose one valid CourseSelection if it doesn't conflict with the already chosen selections.
void backtrack(int index, vector<CourseSelection>& current, const vector<vector<CourseSelection>>& courseOptions) {
    if (index == courseOptions.size()) {
        Schedule sch;
        sch.selections = current;
        allPossibleSchedules.push_back(sch);
        return;
    }
    for (const auto& cs : courseOptions[index]) {
        if (noConflict(current, cs)) {
            current.push_back(cs);
            backtrack(index + 1, current, courseOptions);
            current.pop_back();
        }
    }
}
