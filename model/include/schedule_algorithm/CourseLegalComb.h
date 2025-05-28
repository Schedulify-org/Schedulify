#ifndef COURSELEGALCOMB_H
#define COURSELEGALCOMB_H

#include "model_interfaces.h"
#include "main/inner_structs.h"
#include "parsers/parseCoursesToVector.h"
#include "TimeUtils.h"

class CourseLegalComb {
public:
    static vector<CourseSelection> generate(const Course& course) ;
private:
    static bool hasGroupConflict(const Group* group1, const Group* group2) ;
};
#endif