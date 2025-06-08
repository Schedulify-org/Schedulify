#include <gtest/gtest.h>
#include <sstream>

//// Helper functions for tests
//Course createCourse(int id, const string& raw_id, const string& name) {
//    return Course{id, raw_id, name};
//}
//
//Session* createSession(int day, const string& start, const string& end, const string& building, const string& room) {
//    return new Session{day, start, end, building, room};
//}
//
//Schedule createScheduleWithOneSelection(int courseId, Session* lec, Session* tut, Session* lab) {
//    CourseSelection cs{courseId, lec, tut, lab};
//    return Schedule{{cs}};
//}
//
//// Test for dayToString function
//TEST(postParserTest, ValidAndInvalidDays) {
//    EXPECT_EQ(dayToString(1), "sunday");
//    EXPECT_EQ(dayToString(7), "saturday");
//    EXPECT_EQ(dayToString(0), "unknown");
//    EXPECT_EQ(dayToString(8), "unknown");
//}
//
//// Modified to use buildCourseInfoMap instead of getCourseInfoById
//TEST(postParserTest, FoundAndNotFound) {
//    vector<Course> courses = {
//            createCourse(1, "M101", "Math 101"),
//            createCourse(2, "CS101", "Intro to CS")
//    };
//
//    auto courseInfoMap = buildCourseInfoMap(courses);
//
//    // Test existing course
//    ASSERT_TRUE(courseInfoMap.count(1) > 0);
//    CourseInfo info1 = courseInfoMap.at(1);
//    EXPECT_EQ(info1.name, "Math 101");
//    EXPECT_EQ(info1.raw_id, "M101");
//
//    // Test non-existing course (using the pattern from your code)
//    CourseInfo info2 = courseInfoMap.count(99) ?
//                       courseInfoMap.at(99) : CourseInfo{to_string(99), to_string(99)};
//    EXPECT_EQ(info2.name, "99");
//    EXPECT_EQ(info2.raw_id, "99");
//}
//
//TEST(postParserTest, ValidAndNullSessions) {
//    unordered_map<int, vector<ScheduleItem>> dayMap;
//    CourseInfo courseInfo = {"M101", "Math"};
//
//    // Test null session
//    addSessionToDayMap(dayMap, nullptr, "lecture", courseInfo);
//    EXPECT_TRUE(dayMap.empty());
//
//    // Test valid session
//    auto* session = createSession(1, "10:00", "11:00", "5", "101");
//    addSessionToDayMap(dayMap, session, "lecture", courseInfo);
//    ASSERT_EQ(dayMap.size(), 1);
//    EXPECT_EQ(dayMap[1][0].start, "10:00");
//
//    delete session;
//}
//
//TEST(postParserTest, SingleCourseSelection) {
//    vector<Course> courses = { createCourse(1, "M101", "Math 101") };
//    auto courseInfoMap = buildCourseInfoMap(courses);
//
//    Session* lec = createSession(1, "09:00", "10:00", "2", "200");
//    Schedule schedule = createScheduleWithOneSelection(1, lec, nullptr, nullptr);
//
//    auto dayMap = buildDayMap(schedule, courseInfoMap);
//
//    ASSERT_EQ(dayMap.size(), 7);
//    ASSERT_EQ(dayMap.count(1), 1);
//    EXPECT_EQ(dayMap[1][0].courseName, "Math 101");
//
//    delete lec;
//}
//
//// Modified to test createScheduleDay function
//TEST(postParserTest, SortedSessionsOutput) {
//    unordered_map<int, vector<ScheduleItem>> dayMap;
//    dayMap[1] = {
//            {"Course A", "C001", "lecture", "10:00", "11:00", "1", "101"},
//            {"Course A", "C001", "lecture", "08:00", "09:00", "1", "101"}
//    };
//
//    ScheduleDay day = createScheduleDay(1, dayMap);
//
//    ASSERT_EQ(day.day, "sunday");
//    ASSERT_EQ(day.day_items.size(), 2);
//    // Check that items are sorted by start time
//    EXPECT_EQ(day.day_items[0].start, "08:00");
//    EXPECT_EQ(day.day_items[1].start, "10:00");
//}
//
//// Modified to test InformativeSchedule creation
//TEST(postParserTest, OneScheduleOutput) {
//    vector<Course> courses = { createCourse(1, "PHY101", "Physics 101") };
//    Session* lec = createSession(1, "09:00", "10:00", "3", "305");
//    Schedule sched = createScheduleWithOneSelection(1, lec, nullptr, nullptr);
//
//    auto courseInfoMap = buildCourseInfoMap(courses);
//    InformativeSchedule infoSched = createInformativeSchedule(sched, 1, courseInfoMap);
//
//    ASSERT_FALSE(infoSched.week.empty());
//    EXPECT_EQ(infoSched.index, 1);
//    EXPECT_EQ(infoSched.week[0].day, "sunday");
//    ASSERT_FALSE(infoSched.week[0].day_items.empty());
//    EXPECT_EQ(infoSched.week[0].day_items[0].courseName, "Physics 101");
//
//    delete lec;
//}
//
//// Modified to test the entire export process
//TEST(postParserTest, MultipleSchedules) {
//    vector<Course> courses = { createCourse(1, "ENG101", "English") };
//    Session* lec1 = createSession(1, "08:00", "09:00", "1", "201");
//    Session* lec2 = createSession(2, "10:00", "11:00", "1", "202");
//
//    Schedule sched1 = createScheduleWithOneSelection(1, lec1, nullptr, nullptr);
//    Schedule sched2 = createScheduleWithOneSelection(1, lec2, nullptr, nullptr);
//
//    vector<InformativeSchedule> infoSchedules = exportSchedulesToObjects({sched1, sched2}, courses);
//
//    ASSERT_EQ(infoSchedules.size(), 2);
//    EXPECT_EQ(infoSchedules[0].index, 1);
//    EXPECT_EQ(infoSchedules[1].index, 2);
//
//    // Check first schedule
//    ASSERT_FALSE(infoSchedules[0].week[0].day_items.empty());
//    EXPECT_EQ(infoSchedules[0].week[0].day_items[0].courseName, "English");
//
//    // Check second schedule
//    ASSERT_FALSE(infoSchedules[1].week[1].day_items.empty());
//    EXPECT_EQ(infoSchedules[1].week[1].day_items[0].courseName, "English");
//
//    delete lec1;
//    delete lec2;
//}