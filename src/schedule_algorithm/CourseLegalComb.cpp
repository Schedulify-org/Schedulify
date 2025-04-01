#include "algorithm/CourseLegalComb.h"
#include "algorithm/TimeUtils.h"

vector<CourseSelection> CourseLegalComb::generate(const Course& course) const {
    vector<CourseSelection> combinations;

    for (const auto& lecture : course.Lectures) {
        const Session* lecPtr = &lecture;

        vector<const Session*> tutorials;
        if (course.Tirgulim.empty())
            tutorials.push_back(nullptr);
        else {
            for (const auto& t : course.Tirgulim)
                tutorials.push_back(&t);
        }

        vector<const Session*> labs;
        if (course.labs.empty())
            labs.push_back(nullptr);
        else {
            for (const auto& l : course.labs)
                labs.push_back(&l);
        }

        for (const auto* tutorial : tutorials) {
            if (tutorial && TimeUtils::isOverlap(lecPtr, tutorial)) continue;
            for (const auto* lab : labs) {
                if ((lab && TimeUtils::isOverlap(lecPtr, lab)) || (tutorial && lab && TimeUtils::isOverlap(tutorial, lab)))
                    continue;
                combinations.push_back({course.id, lecPtr, tutorial, lab});
            }
        }
    }

    return combinations;
}