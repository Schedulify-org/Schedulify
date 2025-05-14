#ifndef SCHED_TESTS_GETSESSION_H
#define SCHED_TESTS_GETSESSION_H

#include "model_interfaces.h"
#include "main/main_model.h"

#pragma once

#include <vector>

inline std::vector<const Session*> getSessions(const CourseSelection& cs) {
    std::vector<const Session*> sessions;
    if (cs.lecture) sessions.push_back(cs.lecture);
    if (cs.tutorial) sessions.push_back(cs.tutorial);
    if (cs.lab) sessions.push_back(cs.lab);
    return sessions;
}


#endif //SCHED_TESTS_GETSESSION_H
