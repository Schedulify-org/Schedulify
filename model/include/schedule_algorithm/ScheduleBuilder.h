#ifndef SCHEDULE_BUILDER_H
#define SCHEDULE_BUILDER_H

#include "model_interfaces.h"
#include "CourseLegalComb.h"
#include "inner_structs.h"
#include "getSession.h"
#include "TimeUtils.h"
#include "logger.h"

#include <vector>

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
#endif // SCHEDULE_BUILDER_H