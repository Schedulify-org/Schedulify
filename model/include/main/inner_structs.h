#ifndef INNER_STRUCTS_H
#define INNER_STRUCTS_H

#include "model_interfaces.h"

struct CourseSelection {
    int courseId;
    const Group* lecture;    // Changed from Session* to Group*
    const Group* tutorial;   // Changed from Session* to Group* (nullptr if none)
    const Group* lab;        // Changed from Session* to Group* (nullptr if none)

    // Constructor
    CourseSelection(int id, const Group* lec, const Group* tut = nullptr, const Group* l = nullptr)
        : courseId(id), lecture(lec), tutorial(tut), lab(l) {}
};

struct Schedule {
    vector<CourseSelection> selections;
};

struct CourseInfo {
    string raw_id;
    string name;
};

#endif //INNER_STRUCTS_H