#ifndef COURSELEGALCOMB_H
#define COURSELEGALCOMB_H

#include <vector>
#include "preParser.h"
#include "TimeUtils.h"
#include "schedule_types.h"

class CourseLegalComb {
public:
    vector<CourseSelection> generate(const Course& course) const;
};
#endif 