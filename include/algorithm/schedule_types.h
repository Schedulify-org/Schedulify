#ifndef SCHEDULE_TYPES_H
#define SCHEDULE_TYPES_H

#include <vector>
#include "parsers/preParser.h"

struct CourseSelection {
    int courseId;
    const Session* lecture;
    const Session* tutorial; // nullptr if none
    const Session* lab;      // nullptr if none
};

struct Schedule {
    vector<CourseSelection> selections;
};

#endif
