#ifndef TIMEUTILS_H
#define TIMEUTILS_H
#include <string>
#include "fileHandlers/preParser.h"
#include "schedule_types.h"


class TimeUtils {
public:
    static int toMinutes(const std::string& time);
    static bool isOverlap(const Session* s1, const Session* s2);
};
#endif