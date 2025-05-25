#ifndef GETSESSION_H
#define GETSESSION_H

#include "main/inner_structs.h"
#include "model_interfaces.h"
#include <vector>

// Inline function to extract all individual sessions from a CourseSelection
inline std::vector<const Session*> getSessions(const CourseSelection& cs) {
    std::vector<const Session*> sessions;

    // Add all sessions from lecture group
    if (cs.lecture) {
        for (const auto& session : cs.lecture->sessions) {
            sessions.push_back(&session);
        }
    }

    // Add all sessions from tutorial group
    if (cs.tutorial) {
        for (const auto& session : cs.tutorial->sessions) {
            sessions.push_back(&session);
        }
    }

    // Add all sessions from lab group
    if (cs.lab) {
        for (const auto& session : cs.lab->sessions) {
            sessions.push_back(&session);
        }
    }

    return sessions;
}

#endif // GETSESSION_Hx