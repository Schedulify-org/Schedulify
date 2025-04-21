#include "schedule_algorithm/CourseLegalComb.h"
#include "schedule_algorithm/TimeUtils.h"
#include "parsers/parseCoursesToVector.h"
#include "logs/logger.h"

// Generates all valid combinations of sessions (lecture, tutorial, lab) for a given course
vector<CourseSelection> CourseLegalComb::generate(const Course& course) const {
    vector<CourseSelection> combinations; // Resulting combinations of valid sessions

    try {
        // Iterate over all lecture sessions in the course
        for (const auto& lecture : course.Lectures) {
            const Session* lecPtr = &lecture; // Pointer to the current lecture session

            // Defensive check: Ensure the pointer is not null (redundant here, but safe)
            if (!lecPtr) {
                Logger::get().logWarning("Null lecture session pointer encountered. Skipping.");
                continue;
            }

            // Prepare list of tutorial session pointers
            vector<const Session*> tutorials;
            if (course.Tirgulim.empty()) {
                // If no tutorials exist, allow null (optional) tutorial
                tutorials.push_back(nullptr);
            } else {
                // Add all tutorial session pointers
                for (const auto& t : course.Tirgulim) {
                    tutorials.push_back(&t);
                }
            }

            // Prepare list of lab session pointers
            vector<const Session*> labs;
            if (course.labs.empty()) {
                // If no labs exist, allow null (optional) lab
                labs.push_back(nullptr);
            } else {
                // Add all lab session pointers
                for (const auto& l : course.labs) {
                    labs.push_back(&l);
                }
            }

            // Iterate over all tutorial options for the current lecture
            for (const auto* tutorial : tutorials) {
                // Check if the tutorial conflicts with the lecture
                if (tutorial && TimeUtils::isOverlap(lecPtr, tutorial)) {
                    Logger::get().logInfo("Skipped due to lecture-tutorial conflict for course ID " + to_string(course.id));
                    continue;
                }

                // Iterate over all lab options for the current lecture-tutorial combination
                for (const auto* lab : labs) {
                    // Check if there's a conflict between lecture-lab or tutorial-lab
                    if ((lab && TimeUtils::isOverlap(lecPtr, lab)) ||
                        (tutorial && lab && TimeUtils::isOverlap(tutorial, lab))) {
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
