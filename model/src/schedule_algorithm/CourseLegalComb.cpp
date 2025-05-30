#include "CourseLegalComb.h"

// Generates all valid combinations of groups (lecture, tutorial, lab) for a given course
vector<CourseSelection> CourseLegalComb::generate(const Course& course) {
    vector<CourseSelection> combinations;

    try {
        if (!course.blocks.empty()){
            const Group* blockGroupPtr = &course.blocks[0];
            combinations.push_back({course.id, nullptr, nullptr, nullptr, blockGroupPtr});
        } else {
            for (const auto& lectureGroup : course.Lectures) {
                const Group* lecGroupPtr = &lectureGroup;

                if (!lecGroupPtr) {
                    Logger::get().logWarning("Null lecture group pointer encountered. Skipping.");
                    continue;
                }

                vector<const Group*> tutorials;
                if (course.Tirgulim.empty()) {
                    tutorials.push_back(nullptr);
                } else {
                    for (const auto& t : course.Tirgulim) {
                        tutorials.push_back(&t);
                    }
                }

                vector<const Group*> labs;
                if (course.labs.empty()) {
                    labs.push_back(nullptr);
                } else {
                    for (const auto& l : course.labs) {
                        labs.push_back(&l);
                    }
                }

                for (const auto* tutorialGroup : tutorials) {
                    if (tutorialGroup && hasGroupConflict(lecGroupPtr, tutorialGroup)) {
                        Logger::get().logInfo("Skipped due to lecture-tutorial group conflict for course ID " + to_string(course.id));
                        continue;
                    }

                    for (const auto* labGroup : labs) {
                        if ((labGroup && hasGroupConflict(lecGroupPtr, labGroup)) ||
                            (tutorialGroup && labGroup && hasGroupConflict(tutorialGroup, labGroup))) {
                            Logger::get().logInfo("Skipped due to time conflict in groups for course ID " + to_string(course.id));
                            continue;
                        }

                        combinations.push_back({course.id, lecGroupPtr, tutorialGroup, labGroup, nullptr});
                    }
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