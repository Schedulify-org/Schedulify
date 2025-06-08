#include "ScheduleBuilder.h"
#include "gtest/gtest.h"
#include "test_helpers.h"
#include "parseCoursesToVector.h"


//using namespace std;
//
//// Helper to create a course with given integer ID and sessions
//Course makeCourse(int id, const vector<Session>& lectures,
//                  const vector<Session>& tirgulim = {}, const vector<Session>& labs = {}) {
//    return Course{.id = id, .Lectures = lectures, .Tirgulim = tirgulim, .labs = labs};
//}
//
//// --- TEST CASES ---
//
//// One course with lecture and tutorial that do not conflict
//TEST(ScheduleBuilderTest, OneCourse_NoConflictWithinCourse) {
//    ScheduleBuilder builder;
//
//    Course course = makeCourse(101,
//                               {makeSession(1, "09:00", "10:00")},  // Lecture
//                               {makeSession(1, "10:00", "11:00")}   // Tirgul
//    );
//
//    vector<Schedule> result = builder.build({course});
//    ASSERT_EQ(result.size(), 1);  // One valid schedule
//}
//
//// Two courses with overlapping sessions → conflict
//TEST(ScheduleBuilderTest, TwoCourses_WithConflict) {
//    ScheduleBuilder builder;
//
//    Course courseA = makeCourse(101,
//                                {makeSession(1, "09:00", "11:00")});  // Lecture
//
//    Course courseB = makeCourse(102,
//                                {makeSession(1, "10:00", "12:00")});  // Conflicts with courseA
//
//    vector<Schedule> result = builder.build({courseA, courseB});
//    ASSERT_EQ(result.size(), 0);  // No valid schedules due to conflict
//}
//
//// Two non-overlapping courses → valid combination
//TEST(ScheduleBuilderTest, TwoCourses_NoConflict) {
//    ScheduleBuilder builder;
//
//    Course courseA = makeCourse(101,
//                                {makeSession(1, "09:00", "10:00")});  // Lecture
//
//    Course courseB = makeCourse(102,
//                                {makeSession(1, "10:00", "11:00")});  // Non-conflicting
//
//    vector<Schedule> result = builder.build({courseA, courseB});
//    ASSERT_EQ(result.size(), 1);  // One valid schedule
//}
//
//// Course with only a lecture (no tutorial or lab)
//TEST(ScheduleBuilderTest, NoTutorialOrLab) {
//    ScheduleBuilder builder;
//
//    Course course = makeCourse(201, {makeSession(2, "13:00", "15:00")});
//    vector<Schedule> result = builder.build({course});
//    ASSERT_EQ(result.size(), 1);  // Should handle lack of tutorials/labs
//}
//
//// Two courses with multiple valid session combinations (no conflicts)
//TEST(ScheduleBuilderTest, MultipleCombinations) {
//    ScheduleBuilder builder;
//
//    Course courseA = makeCourse(301, {
//            makeSession(1, "09:00", "10:00"),
//            makeSession(1, "10:00", "11:00")
//    });
//
//    Course courseB = makeCourse(302, {
//            makeSession(2, "09:00", "10:00"),
//            makeSession(2, "10:00", "11:00")
//    });
//
//    vector<Schedule> result = builder.build({courseA, courseB});
//    ASSERT_EQ(result.size(), 4);  // 2 lectures per course = 2x2 = 4 valid combos
//}
//
//// Edge case: no courses given
//TEST(ScheduleBuilderTest, EmptyCourseList) {
//    ScheduleBuilder builder;
//    vector<Schedule> result = builder.build({});
//    ASSERT_EQ(result.size(), 1);  // One "empty" valid schedule (base case)
//}
//
//// One course with a single lecture session
//TEST(ScheduleBuilderTest, OneCourse_OneSessionOnly) {
//    ScheduleBuilder builder;
//    Course course = makeCourse(501, {makeSession(0, "08:00", "09:00")});
//    vector<Schedule> result = builder.build({course});
//    ASSERT_EQ(result.size(), 1);  // Only one option, should be valid
//}
//
//// Three courses with identical session times → all conflict
//TEST(ScheduleBuilderTest, MultipleCourses_ExactSameTimes) {
//    ScheduleBuilder builder;
//
//    auto session = makeSession(1, "09:00", "10:00");
//    Course course1 = makeCourse(601, {session});
//    Course course2 = makeCourse(602, {session});
//    Course course3 = makeCourse(603, {session});
//
//    vector<Schedule> result = builder.build({course1, course2, course3});
//    ASSERT_EQ(result.size(), 0);  // All conflict with each other
//}
//
//// Courses without tutorials or labs → test that nullptrs are handled correctly
//TEST(ScheduleBuilderTest, CoursesWithNoTutorialOrLab_NullptrHandled) {
//    ScheduleBuilder builder;
//
//    Course course1 = makeCourse(701, {makeSession(3, "12:00", "13:00")});
//    Course course2 = makeCourse(702, {makeSession(3, "13:00", "14:00")});
//
//    vector<Schedule> result = builder.build({course1, course2});
//    ASSERT_EQ(result.size(), 1);  // Should produce one valid schedule
//}
//
//// Ten non-conflicting courses in sequence → large input test
//TEST(ScheduleBuilderTest, LargeInput_NoConflicts) {
//    ScheduleBuilder builder;
//
//    vector<Course> manyCourses;
//    for (int i = 0; i < 10; ++i) {
//        // Each course starts an hour after the previous
//        int hour = 8 + i;
//        string start = to_string(hour) + ":00";
//        string end = to_string(hour + 1) + ":00";
//        manyCourses.push_back(makeCourse(800 + i, {makeSession(1, start, end)}));
//    }
//
//    vector<Schedule> result = builder.build(manyCourses);
//    ASSERT_EQ(result.size(), 1);  // One valid schedule including all 10 courses
//}
//
//// Course Without Lecture Should Be Skipped
//TEST(ScheduleBuilderTest, CourseWithoutLectureShouldBeSkipped) {
//    ScheduleBuilder builder;
//
//    // Course has tutorial and lab, but no lecture
//    Course course = makeCourse(1001, {},
//                               {makeSession(1, "10:00", "11:00")},
//                               {makeSession(1, "11:00", "12:00")}
//    );
//
//    vector<Schedule> result = builder.build({course});
//    ASSERT_EQ(result.size(), 0);  // No valid schedule since lecture is required
//}
//
//// Chained Conflicts Block All Schedules
//TEST(ScheduleBuilderTest, ChainedConflictsBlocksAllSchedules) {
//    ScheduleBuilder builder;
//
//    Course a = makeCourse(1101, {makeSession(1, "09:00", "10:00")});
//    Course b = makeCourse(1102, {makeSession(1, "09:30", "10:30")});  // Overlaps with A and C
//    Course c = makeCourse(1103, {makeSession(1, "10:00", "11:00")});
//
//    vector<Schedule> result = builder.build({a, b, c});
//    ASSERT_EQ(result.size(), 0);  // All schedules blocked due to transitive conflict
//}
//
//// Schedule Contains Correct Times
//TEST(ScheduleBuilderTest, ScheduleContainsCorrectTimes) {
//    ScheduleBuilder builder;
//
//    Course courseA = makeCourse(1301, {makeSession(1, "09:00", "10:00")});
//    Course courseB = makeCourse(1302, {makeSession(1, "10:00", "11:00")});
//
//    vector<Schedule> result = builder.build({courseA, courseB});
//    ASSERT_EQ(result.size(), 1);  // Should succeed
//
//    const Schedule& sched = result[0];
//    ASSERT_EQ(sched.selections.size(), 2);  // One selection per course
//}
