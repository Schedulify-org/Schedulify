#ifndef GET_SESSION_H
#define GET_SESSION_H

#pragma once

#include "model_interfaces.h"
#include "inner_structs.h"

#include <vector>

inline vector<const Session*> getSessions(const CourseSelection& cs) {
    vector<const Session*> sessions;

    if (cs.lectureGroup) {
        for (const auto& session : cs.lectureGroup->sessions) {
            sessions.push_back(&session);
        }
    }
    if (cs.tutorialGroup) {
        for (const auto& session : cs.tutorialGroup->sessions) {
            sessions.push_back(&session);
        }
    }
    if (cs.labGroup) {
        for (const auto& session : cs.labGroup->sessions) {
            sessions.push_back(&session);
        }
    }
    if (cs.blockGroup) {
        for (const auto& session : cs.blockGroup->sessions) {
            sessions.push_back(&session);
        }
    }
    return sessions;
}


#endif //GET_SESSION_H