#ifndef SCHEDULE_BUILDER_H
#define SCHEDULE_BUILDER_H

#include "model_interfaces.h"
#include "main/inner_structs.h"
#include "schedule_algorithm/CourseLegalComb.h"
#include "schedule_algorithm/TimeUtils.h"
#include "schedule_algorithm/getSession.h"
#include "logger/logger.h"

#include <unordered_map>
#include <algorithm>
#include <vector>
#include <map>

class ScheduleBuilder {
public:
    vector<InformativeSchedule> build(const vector<Course>& courses);

private:
    static unordered_map<int, CourseInfo> courseInfoMap;

    void backtrack(
            int index,
            const vector<vector<CourseSelection>>& allOptions,
            vector<CourseSelection>& current,
            vector<InformativeSchedule>& results);

    static bool hasConflict(const CourseSelection& a, const CourseSelection& b) ;

    InformativeSchedule convertToInformativeSchedule(const vector<CourseSelection>& selections, int index) const;

    static void processGroupSessions(const CourseSelection& selection,
                              const Group* group,
                              const string& sessionType,
                              map<int, vector<ScheduleItem>>& daySchedules) ;

    static string getCourseNameById(int courseId) ;

    static string getCourseRawIdById(int courseId) ;

    static void buildCourseInfoMap(const vector<Course>& courses);
};

#endif // SCHEDULE_BUILDER_H