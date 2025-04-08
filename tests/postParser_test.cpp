#include <gtest/gtest.h>
#include <sstream>
#include "parsers/postParser.h"  // Your header file

Course createCourse(int id, const string& raw_id, const string& name) {
    return Course{id, raw_id, name};
}

Session* createSession(int day, const string& start, const string& end, const string& building, const string& room) {
    return new Session{day, start, end, building, room};
}

Schedule createScheduleWithOneSelection(int courseId, Session* lec, Session* tut, Session* lab) {
    CourseSelection cs{courseId, lec, tut, lab};
    return Schedule{{cs}};
}

TEST(DayToStringTest, ValidAndInvalidDays) {
    EXPECT_EQ(dayToString(1), "sunday");
    EXPECT_EQ(dayToString(7), "saturday");
    EXPECT_EQ(dayToString(0), "unknown");
    EXPECT_EQ(dayToString(8), "unknown");
}

TEST(GetCourseInfoTest, FoundAndNotFound) {
    vector<Course> courses = {
        createCourse(1, "M101", "Math 101"),
        createCourse(2, "CS101", "Intro to CS")
    };

    CourseInfo info1 = getCourseInfoById(courses, 1);
    EXPECT_EQ(info1.name, "Math 101");
    EXPECT_EQ(info1.raw_id, "M101");

    CourseInfo info2 = getCourseInfoById(courses, 99);
    EXPECT_EQ(info2.name, "99");
    EXPECT_EQ(info2.raw_id, "99");
}

TEST(AddSessionToDayMapTest, ValidAndNullSessions) {
    unordered_map<int, vector<ScheduleItem>> dayMap;

    addSessionToDayMap(dayMap, nullptr, "lecture", "Math", "M101");
    EXPECT_TRUE(dayMap.empty());

    auto* session = createSession(1, "10:00", "11:00", "5", "101");
    addSessionToDayMap(dayMap, session, "lecture", "Math", "M101");
    ASSERT_EQ(dayMap.size(), 1);
    EXPECT_EQ(dayMap[1][0].start, "10:00");

    delete session;
}

TEST(BuildDayMapTest, SingleCourseSelection) {
    vector<Course> courses = { createCourse(1, "M101", "Math 101") };
    Session* lec = createSession(1, "09:00", "10:00", "2", "200");
    Schedule schedule = createScheduleWithOneSelection(1, lec, nullptr, nullptr);

    auto map = buildDayMapForSchedule(schedule, courses);

    ASSERT_EQ(map.size(), 1);
    ASSERT_EQ(map.count(1), 1);
    EXPECT_EQ(map[1][0].courseName, "Math 101");

    delete lec;
}

TEST(WriteDayScheduleToFileTest, SortedSessionsOutput) {
    vector<ScheduleItem> items = {
        {"Course A", "C001", "lecture", "10:00", "11:00", "1", "101"},
        {"Course A", "C001", "lecture", "08:00", "09:00", "1", "101"},
    };

    std::stringstream ss;
    writeDayScheduleToFile(ss, 1, items);
    string output = ss.str();

    size_t pos1 = output.find("08:00");
    size_t pos2 = output.find("10:00");
    EXPECT_NE(pos1, string::npos);
    EXPECT_NE(pos2, string::npos);
    EXPECT_LT(pos1, pos2);
}

TEST(WriteScheduleToFileTest, OneScheduleOutput) {
    vector<Course> courses = { createCourse(1, "PHY101", "Physics 101") };
    Session* lec = createSession(1, "09:00", "10:00", "3", "305");
    Schedule sched = createScheduleWithOneSelection(1, lec, nullptr, nullptr);

    std::stringstream ss;
    writeScheduleToFile(ss, sched, 0, courses);
    string output = ss.str();

    EXPECT_NE(output.find("schedule 1:"), string::npos);
    EXPECT_NE(output.find("sunday:"), string::npos);
    EXPECT_NE(output.find("Physics 101"), string::npos);

    delete lec;
}

TEST(ExportSchedulesTest, MultipleSchedules) {
    vector<Course> courses = { createCourse(1, "ENG101", "English") };
    Session* lec1 = createSession(1, "08:00", "09:00", "1", "201");
    Session* lec2 = createSession(2, "10:00", "11:00", "1", "202");

    Schedule sched1 = createScheduleWithOneSelection(1, lec1, nullptr, nullptr);
    Schedule sched2 = createScheduleWithOneSelection(1, lec2, nullptr, nullptr);

    string path = "temp_test_output.txt";
    exportSchedulesToText({sched1, sched2}, path, courses);

    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    string output = buffer.str();

    EXPECT_NE(output.find("schedule 1:"), string::npos);
    EXPECT_NE(output.find("schedule 2:"), string::npos);
    EXPECT_NE(output.find("English"), string::npos);

    file.close();
    std::remove(path.c_str()); // Clean up

    delete lec1;
    delete lec2;
}
