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
        if (course.Lectures.empty()) {
            Logger::get().logWarning("Course ID " + to_string(course.id) + " has no lecture groups. Skipping course.");
            return combinations; // Return empty if no lectures available
        }

        // Iterate over all lecture groups in the course
        for (const auto& lectureGroup : course.Lectures) {
            const Group* lecPtr = &lectureGroup; // Pointer to the current lecture group

            // Defensive check: Ensure the pointer is not null
            if (!lecPtr) {
                Logger::get().logWarning("Null lecture group pointer encountered. Skipping.");
                continue;
            }

            // CRITICAL FIX: Ensure lecture group has sessions
            if (lecPtr->sessions.empty()) {
                Logger::get().logWarning("Lecture group has no sessions for course ID " + to_string(course.id) + ". Skipping.");
                continue;
            }

            // Prepare list of tutorial group pointers
            vector<const Group*> tutorials;
            if (course.Tirgulim.empty()) {
                // If no tutorials exist, allow null (optional) tutorial
                tutorials.push_back(nullptr);
            } else {
                // Add all tutorial group pointers, but only those with sessions
                for (const auto& t : course.Tirgulim) {
                    if (!t.sessions.empty()) {  // FIXED: Only add tutorials with sessions
                        tutorials.push_back(&t);
                    }
                }
                // If no valid tutorials found, add nullptr to allow lecture-only combinations
                if (tutorials.empty()) {
                    tutorials.push_back(nullptr);
                }
            }

            // Prepare list of lab group pointers
            vector<const Group*> labs;
            if (course.labs.empty()) {
                // If no labs exist, allow null (optional) lab
                labs.push_back(nullptr);
            } else {
                // Add all lab group pointers, but only those with sessions
                for (const auto& l : course.labs) {
                    if (!l.sessions.empty()) {  // FIXED: Only add labs with sessions
                        labs.push_back(&l);
                    }
                }
                // If no valid labs found, add nullptr to allow combinations without labs
                if (labs.empty()) {
                    labs.push_back(nullptr);
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

                    // CRITICAL FIX: Always ensure we have a lecture in every combination
                    // If no conflicts, add this combination as valid (lecture is mandatory)
                    combinations.push_back({course.id, lecPtr, tutorial, lab});
                    Logger::get().logInfo("Added valid combination for course ID " + to_string(course.id) +
                                          " - Lecture: YES, Tutorial: " + (tutorial ? "YES" : "NO") +
                                          ", Lab: " + (lab ? "YES" : "NO"));
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