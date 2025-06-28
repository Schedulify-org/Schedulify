#ifndef SCHEDULE_BUILDER_H
#define SCHEDULE_BUILDER_H

#include "model_interfaces.h"
#include "CourseLegalComb.h"
#include "inner_structs.h"
#include "getSession.h"
#include "TimeUtils.h"
#include "logger.h"
#include "ScheduleDatabaseWriter.h"

#include <unordered_map>
#include <algorithm>
#include <vector>
#include <map>

class ScheduleBuilder {
public:
    // Build schedules with optional progressive DB writing
    vector<InformativeSchedule> build(const vector<Course>& courses, bool writeToDatabase = false,
                                      const string& setName = "", const vector<int>& sourceFileIds = {});

private:
    static unordered_map<int, CourseInfo> courseInfoMap;

    // Progressive writing state
    bool progressiveWriting = false;
    int totalSchedulesGenerated = 0;

    void backtrack(int index, const vector<vector<CourseSelection>>& allOptions, vector<CourseSelection>& current,
            vector<InformativeSchedule>& results);

    static bool hasConflict(const CourseSelection& a, const CourseSelection& b);

    static InformativeSchedule convertToInformativeSchedule(const vector<CourseSelection>& selections, int index);

    static void processGroupSessions(const CourseSelection& selection, const Group* group, const string& sessionType,
                                     map<int, vector<ScheduleItem>>& daySchedules);

    static string getCourseNameById(int courseId);
    static string getCourseRawIdById(int courseId);
    static void buildCourseInfoMap(const vector<Course>& courses);
    static void calculateScheduleMetrics(InformativeSchedule& schedule);
};

#endif // SCHEDULE_BUILDER_H