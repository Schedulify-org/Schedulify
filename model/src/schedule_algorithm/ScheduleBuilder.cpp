#include "schedule_algorithm/ScheduleBuilder.h"
#include "logs/logger.h"

using namespace std;

// Checks if there is a time conflict between two CourseSelections
bool ScheduleBuilder::hasConflict(const CourseSelection& a, const CourseSelection& b) const {
    vector<const Session*> aSessions = getSessions(a); // Extract sessions from selection a
    vector<const Session*> bSessions = getSessions(b); // Extract sessions from selection b

    // Compare each session in a with each session in b
    for (const auto* s1 : aSessions) {
        for (const auto* s2 : bSessions) {
            // Return true immediately if any overlapping sessions are found
            if (TimeUtils::isOverlap(s1, s2)) return true;
        }
    }
    return false; // No conflicts found
}

// Recursive backtracking function to build all valid schedules
void ScheduleBuilder::backtrack(int currentCourse,
                                const vector<vector<CourseSelection>>& allOptions,
                                vector<CourseSelection>& currentCombination,
                                vector<Schedule>& results) {
    try {
        // Base case: All courses have been processed, save the current schedule
        if (currentCourse == allOptions.size()) {
            results.push_back({currentCombination});
            return;
        }

        // Try each course selection option for the current course
        for (const auto& option : allOptions[currentCourse]) {
            bool conflict = false;

            // Check for conflicts with the selections already made
            for (const auto& selected : currentCombination) {
                if (hasConflict(option, selected)) {
                    conflict = true;
                    break;
                }
            }

            // If no conflict, continue to the next course
            if (!conflict) {
                currentCombination.push_back(option); // Choose this option
                backtrack(currentCourse + 1, allOptions, currentCombination, results); // Recurse
                currentCombination.pop_back(); // Backtrack
            }
        }
    } catch (const exception& e) {
        // Log any exceptions that occur during backtracking
        Logger::get().logError("Exception in ScheduleBuilder::backtrack: " + string(e.what()));
    }
}

// Public method to build all possible valid schedules from a list of courses
vector<Schedule> ScheduleBuilder::build(const vector<Course>& courses) {
    Logger::get().logInfo("Starting schedule generation for " + to_string(courses.size()) + " courses.");

    vector<Schedule> results; // Final list of all valid schedules
    try {
        CourseLegalComb generator; // Helper to generate valid session combinations
        vector<vector<CourseSelection>> allOptions; // All valid options per course

        // Generate combinations for each course
        for (const auto& course : courses) {
            auto combinations = generator.generate(course);
            Logger::get().logInfo("Generated " + to_string(combinations.size()) + " combinations for course ID " + to_string(course.id));
            allOptions.push_back(std::move(combinations)); // Store the combinations
        }

        vector<CourseSelection> current; // Temporary list for current schedule
        backtrack(0, allOptions, current, results); // Start recursive backtracking

        Logger::get().logInfo("Finished schedule generation. Total valid schedules: " + to_string(results.size()));
    } catch (const exception& e) {
        // Log any exceptions that occur during schedule generation
        Logger::get().logError("Exception in ScheduleBuilder::build: " + string(e.what()));
    }

    return results; // Return all valid generated schedules
}
