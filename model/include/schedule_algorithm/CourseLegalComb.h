#ifndef COURSELEGALCOMB_H
#define COURSELEGALCOMB_H

#include "model_interfaces.h"
#include "main/inner_structs.h"
#include "parsers/parseCoursesToVector.h"
#include "TimeUtils.h"

class CourseLegalComb {
public:
    // Generates all valid combinations of session groups for a given course
    vector<CourseSelection> generate(const Course& course) const;

private:
    // Helper function to check if two groups have conflicting sessions
    bool hasGroupConflict(const Group& group1, const Group& group2) const;
};

#endif //COURSELEGALCOMB_H