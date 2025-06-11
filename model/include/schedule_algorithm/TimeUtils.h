#ifndef TIMEUTILS_H
#define TIMEUTILS_H

#include "parseCoursesToVector.h"
#include "model_interfaces.h"
#include "logger.h"

#include <string>

class TimeUtils {
public:
    static int toMinutes(const std::string& time);
    static bool isOverlap(const Session* s1, const Session* s2);
};
#endif