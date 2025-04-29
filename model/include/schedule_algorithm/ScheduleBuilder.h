#ifndef SCHEDULE_BUILDER_H
#define SCHEDULE_BUILDER_H

#include "main_include.h"
#include "parsers/parseCoursesToVector.h"
#include "TimeUtils.h"
#include "schedule_algorithm/CourseLegalComb.h"
#include "schedule_algorithm/TimeUtils.h"
#include "schedule_algorithm/getSession.h"

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