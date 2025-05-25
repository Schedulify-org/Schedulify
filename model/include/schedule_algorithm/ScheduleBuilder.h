#ifndef SCHEDULE_BUILDER_H
#define SCHEDULE_BUILDER_H

#include "model_interfaces.h"
#include "main/inner_structs.h"
#include "schedule_algorithm/CourseLegalComb.h"
#include "schedule_algorithm/TimeUtils.h"
#include "schedule_algorithm/getSession.h"
#include "logger/logger.h"

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

    // Helper function to extract all individual sessions from a CourseSelection
    vector<const Session*> getSessions(const CourseSelection& selection) const;
};
#endif // SCHEDULE_BUILDER_H