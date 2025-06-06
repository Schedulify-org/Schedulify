#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "model_interfaces.h"

inline Session makeSession(int day, const std::string& start, const std::string& end) {
    return Session{.day_of_week = day, .start_time = start, .end_time = end};
}

#endif // TEST_HELPERS_H
