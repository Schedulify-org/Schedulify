#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include "../include/parser.h"
#include "../include/algoritm.h"
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

bool hasConflict(const CourseSelection& a, const CourseSelection& b) {
    std::vector<const Session*> aSessions = {a.lecture, a.tutorial, a.lab};
    std::vector<const Session*> bSessions = {b.lecture, b.tutorial, b.lab};

    for (auto* sa : aSessions) {
        if (!sa) continue;
        for (auto* sb : bSessions) {
            if (!sb) continue;
            if (sa->day_of_week != sb->day_of_week) continue;

            int startA = timeToMinutes(sa->start_time);
            int endA = timeToMinutes(sa->end_time);
            int startB = timeToMinutes(sb->start_time);
            int endB = timeToMinutes(sb->end_time);

            if (startA < endB && startB < endA) return true;
        }
    }
    return false;
}

vector<CourseSelection> generateValidSelections(const Course& course) {
    vector<CourseSelection> results;
    for (auto* lecture : course.lectures) {
        if (!course.tutorials.empty()) {
            for (auto* tutorial : course.tutorials) {
                if (hasConflict({-1, lecture, nullptr, nullptr}, {-1, tutorial, nullptr, nullptr})) continue;
                if (!course.labs.empty()) {
                    for (auto* lab : course.labs) {
                        if (hasConflict({-1, lecture, tutorial, nullptr}, {-1, nullptr, nullptr, lab})) continue;
                        results.push_back({course.courseId, lecture, tutorial, lab});
                    }
                } else {
                    results.push_back({course.courseId, lecture, tutorial, nullptr});
                }
            }
        } else if (!course.labs.empty()) {
            for (auto* lab : course.labs) {
                if (hasConflict({-1, lecture, nullptr, nullptr}, {-1, nullptr, nullptr, lab})) continue;
                results.push_back({course.courseId, lecture, nullptr, lab});
            }
        } else {
            results.push_back({course.courseId, lecture, nullptr, nullptr});
        }
    }
    return results;
}

void generateAllSchedules(int index,
                          vector<CourseSelection>& current,
                          const vector<vector<CourseSelection>>& allOptions,
                          vector<Schedule>& results) {
    if (index == allOptions.size()) {
        results.push_back({current});
        return;
    }

    for (const auto& cs : allOptions[index]) {
        bool conflict = false;
        for (const auto& selected : current) {
            if (hasConflict(selected, cs)) {
                conflict = true;
                break;
            }
        }
        if (!conflict) {
            current.push_back(cs);
            generateAllSchedules(index + 1, current, allOptions, results);
            current.pop_back();
        }
    }
}
