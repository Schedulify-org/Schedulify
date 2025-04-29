#ifndef COURSELEGALCOMB_H
#define COURSELEGALCOMB_H

#include "main_include.h"
#include "parsers/parseCoursesToVector.h"
#include "TimeUtils.h"

class CourseLegalComb {
public:
    vector<CourseSelection> generate(const Course& course) const;
};
#endif 