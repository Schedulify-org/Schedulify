#ifndef INNER_STRUCTS_H
#define INNER_STRUCTS_H

#include "model_interfaces.h"

struct CourseSelection {
    int courseId;
    const Session* lecture;
    const Session* tutorial; // nullptr if none
    const Session* lab;      // nullptr if none
};

struct Schedule {
    vector<CourseSelection> selections;
};

struct CourseInfo {
    string raw_id;
    string name;
};

#endif //INNER_STRUCTS_H
