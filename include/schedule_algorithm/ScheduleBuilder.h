#ifndef SCHEDULEBUILDER_H
#define SCHEDULEBUILDER_H
#include <vector>
#include "parsers/preParser.h"
#include "TimeUtils.h"
#include "schedule_types.h"

class ScheduleBuilder {
public:
    vector<Schedule> build(const vector<Course>& courses);

private:
    void backtrack(
        int index,
        const vector<vector<CourseSelection>>& allOptions,
        vector<CourseSelection>& current,
        vector<Schedule>& results);

    bool hasConflict(const CourseSelection& a, const CourseSelection& b) const;
};
#endif