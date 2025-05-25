#include "schedule_algorithm/CourseLegalComb.h"
#include "schedule_algorithm/TimeUtils.h"
#include "parsers/parseCoursesToVector.h"
#include "logger/logger.h"

// Helper function to check if two groups have any overlapping sessions
bool CourseLegalComb::hasGroupConflict(const Group& group1, const Group& group2) const {
    // Check every session in group1 against every session in group2
    for (const auto& session1 : group1.sessions) {
        for (const auto& session2 : group2.sessions) {
            if (TimeUtils::isOverlap(&session1, &session2)) {
                return true; // Found a conflict
            }
        }
    }
    return false; // No conflicts found
}

// Generates all valid combinations of sessions (lecture, tutorial, lab) for a given course
vector<CourseSelection> CourseLegalComb::generate(const Course& course) const {
    vector<CourseSelection> combinations; // Resulting combinations of valid sessions

    try {
        // Iterate over all lecture groups in the course
        for (const auto& lectureGroup : course.Lectures) {
            const Group* lecPtr = &lectureGroup; // Pointer to the current lecture group

            // Defensive check: Ensure the pointer is not null
            if (!lecPtr) {
                Logger::get().logWarning("Null lecture group pointer encountered. Skipping.");
                continue;
            }

            // Prepare list of tutorial group pointers
            vector<const Group*> tutorials;
            if (course.Tirgulim.empty()) {
                // If no tutorials exist, allow null (optional) tutorial
                tutorials.push_back(nullptr);
            } else {
                // Add all tutorial group pointers
                for (const auto& t : course.Tirgulim) {
                    tutorials.push_back(&t);
                }
            }

            // Prepare list of lab group pointers
            vector<const Group*> labs;
            if (course.labs.empty()) {
                // If no labs exist, allow null (optional) lab
                labs.push_back(nullptr);
            } else {
                // Add all lab group pointers
                for (const auto& l : course.labs) {
                    labs.push_back(&l);
                }
            }

            // Iterate over all tutorial options for the current lecture
            for (const auto* tutorial : tutorials) {
                // Check if the tutorial conflicts with the lecture
                if (tutorial && hasGroupConflict(*lecPtr, *tutorial)) {
                    Logger::get().logInfo("Skipped due to lecture-tutorial conflict for course ID " + to_string(course.id));
                    continue;
                }

                // Iterate over all lab options for the current lecture-tutorial combination
                for (const auto* lab : labs) {
                    // Check if there's a conflict between lecture-lab or tutorial-lab
                    if ((lab && hasGroupConflict(*lecPtr, *lab)) ||
                        (tutorial && lab && hasGroupConflict(*tutorial, *lab))) {
                        Logger::get().logInfo("Skipped due to time conflict in course ID " + to_string(course.id));
                        continue;
                    }

                    // If no conflicts, add this combination as valid
                    combinations.push_back({course.id, lecPtr, tutorial, lab});
                }
            }
        }

        // Log warning if no combinations were found
        if (combinations.empty()) {
            Logger::get().logWarning("No valid combinations generated for course ID " + to_string(course.id));
        } else {
            // Log info on how many combinations were generated
            Logger::get().logInfo("Generated " + to_string(combinations.size()) + " valid combinations for course ID " + to_string(course.id));
        }
    } catch (const exception& e) {
        // Catch and log any unexpected exceptions during generation
        Logger::get().logError("Exception in CourseLegalComb::generate for course ID " + to_string(course.id) + ": " + e.what());
    }

    return combinations; // Return the list of valid combinations
}