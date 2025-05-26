#include "schedule_algorithm/CourseLegalComb.h"
#include "schedule_algorithm/TimeUtils.h"
#include "parsers/parseCoursesToVector.h"
#include "logger/logger.h"

// Generates all valid combinations of groups (lecture, tutorial, lab) for a given course
vector<CourseSelection> CourseLegalComb::generate(const Course& course) {
    vector<CourseSelection> combinations;

    try {
        for (const auto& lectureGroup : course.Lectures) {
            const Group* lecGroupPtr = &lectureGroup; // Pointer to the current lecture group

            if (!lecGroupPtr) {
                Logger::get().logWarning("Null lecture group pointer encountered. Skipping.");
                continue;
            }

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

            for (const auto* tutorialGroup : tutorials) {
                // Check if any session in the tutorial group conflicts with any session in the lecture group
                if (tutorialGroup && hasGroupConflict(lecGroupPtr, tutorialGroup)) {
                    Logger::get().logInfo("Skipped due to lecture-tutorial group conflict for course ID " + to_string(course.id));
                    continue;
                }

                // Iterate over all lab options for the current lecture-tutorial combination
                for (const auto* labGroup : labs) {
                    // Check if there's a conflict between lecture-lab groups or tutorial-lab groups
                    if ((labGroup && hasGroupConflict(lecGroupPtr, labGroup)) ||
                        (tutorialGroup && labGroup && hasGroupConflict(tutorialGroup, labGroup))) {
                        Logger::get().logInfo("Skipped due to time conflict in groups for course ID " + to_string(course.id));
                        continue;
                    }

                    // If no conflicts, add this combination as valid
                    combinations.push_back({course.id, lecGroupPtr, tutorialGroup, labGroup});
                }
            }
        }

        if (combinations.empty()) {
            Logger::get().logWarning("No valid combinations generated for course ID " + to_string(course.id));
        } else {
            Logger::get().logInfo("Generated " + to_string(combinations.size()) + " valid combinations for course ID " + to_string(course.id));
        }
    } catch (const exception& e) {
        Logger::get().logError("Exception in CourseLegalComb::generate for course ID " + to_string(course.id) + ": " + e.what());
    }

    return combinations;
}

// Helper method to check if two groups have any conflicting sessions
bool CourseLegalComb::hasGroupConflict(const Group* group1, const Group* group2) {
    if (!group1 || !group2) {
        return false;
    }

    try {
        for (const auto& session1 : group1->sessions) {
            for (const auto& session2 : group2->sessions) {
                if (TimeUtils::isOverlap(&session1, &session2)) {
                    return true;
                }
            }
        }
    } catch (const exception& e) {
        Logger::get().logError("Exception in hasGroupConflict: " + string(e.what()));
        return true;
    }

    return false; // No conflicts found
}