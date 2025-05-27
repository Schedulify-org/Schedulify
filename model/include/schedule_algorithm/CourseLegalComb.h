#ifndef COURSELEGALCOMB_H
#define COURSELEGALCOMB_H

#include "parseCoursesToVector.h"
#include "model_interfaces.h"
#include "inner_structs.h"
#include "TimeUtils.h"
#include "logger.h"

class CourseLegalComb {
public:
    static vector<CourseSelection> generate(const Course& course) ;
private:
    static bool hasGroupConflict(const Group* group1, const Group* group2) ;
};
#endif