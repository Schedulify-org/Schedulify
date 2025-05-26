#ifndef INNER_STRUCTS_H
#define INNER_STRUCTS_H

#include "model_interfaces.h"

struct CourseSelection {
    int courseId;
    const Group* lectureGroup;
    const Group* tutorialGroup;  // nullptr if none
    const Group* labGroup;       // nullptr if none
};

struct CourseInfo {
    string raw_id;
    string name;
};

#endif //INNER_STRUCTS_H