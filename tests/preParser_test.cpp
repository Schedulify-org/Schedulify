// tests/preParser_test.cpp
#include <gtest/gtest.h>
#include "parsers/preParser.h"
#include <fstream>
#include <filesystem>

using namespace std;

class PreParserTest : public ::testing::Test {
protected:
    string testCourseDBPath = "test_course_db.txt";
    string testUserInputPath = "../data/userInput.txt";

    void SetUp() override {
        // Write dummy course DB
        ofstream db(testCourseDBPath);
        db << "Software Engineering\n"
           << "42\n"
           << "Dr. Who\n"
           << "L S,3,09:00,11:00,BuildA,Room5\n"
           << "$$$$\n";
        db.close();

        // Write user input
        ofstream user(testUserInputPath);
        user << "42\n";
        user.close();
    }

    void TearDown() override {
        remove(testCourseDBPath.c_str());
        remove(testUserInputPath.c_str());
    }
};

TEST_F(PreParserTest, ValidTime) {
EXPECT_TRUE(isValidTime("13:45"));
EXPECT_FALSE(isValidTime("1345"));
EXPECT_FALSE(isValidTime("24:00"));
EXPECT_FALSE(isValidTime("10:70"));
}

TEST_F(PreParserTest, ParsesValidCourseDB) {
auto courses = parseCourseDB(testCourseDBPath);
ASSERT_EQ(courses.size(), 1);
EXPECT_EQ(courses[0].id, 42);
EXPECT_EQ(courses[0].Lectures.size(), 1);
EXPECT_EQ(courses[0].Lectures[0].day_of_week, 3);
}

TEST(SessionTest, InvalidSessionThrows) {
EXPECT_THROW(parseSingleSession("S,2,12:00,11:00,Bld,Room"), std::invalid_argument);
}

TEST(SessionTest, ValidSessionParsedCorrectly) {
Session s = parseSingleSession("S,4,08:30,10:00,B1,202");
EXPECT_EQ(s.day_of_week, 4);
EXPECT_EQ(s.start_time, "08:30");
EXPECT_EQ(s.end_time, "10:00");
}
// tests/preParser_test.cpp
#include <gtest/gtest.h>
#include "parsers/preParser.h"
#include <fstream>
#include <filesystem>

using namespace std;

class PreParserTest : public ::testing::Test {
protected:
    string testCourseDBPath = "test_course_db.txt";
    string testUserInputPath = "../data/userInput.txt";

    void SetUp() override {
        // Write dummy course DB
        ofstream db(testCourseDBPath);
        db << "Software Engineering\n"
           << "42\n"
           << "Dr. Who\n"
           << "L S,3,09:00,11:00,BuildA,Room5\n"
           << "$$$$\n";
        db.close();

        // Write user input
        ofstream user(testUserInputPath);
        user << "42\n";
        user.close();
    }

    void TearDown() override {
        remove(testCourseDBPath.c_str());
        remove(testUserInputPath.c_str());
    }
};

TEST_F(PreParserTest, ValidTime) {
EXPECT_TRUE(isValidTime("13:45"));
EXPECT_FALSE(isValidTime("1345"));
EXPECT_FALSE(isValidTime("24:00"));
EXPECT_FALSE(isValidTime("10:70"));
}

TEST_F(PreParserTest, ParsesValidCourseDB) {
auto courses = parseCourseDB(testCourseDBPath);
ASSERT_EQ(courses.size(), 1);
EXPECT_EQ(courses[0].id, 42);
EXPECT_EQ(courses[0].Lectures.size(), 1);
EXPECT_EQ(courses[0].Lectures[0].day_of_week, 3);
}

TEST(SessionTest, InvalidSessionThrows) {
EXPECT_THROW(parseSingleSession("S,2,12:00,11:00,Bld,Room"), std::invalid_argument);
}

TEST(SessionTest, ValidSessionParsedCorrectly) {
Session s = parseSingleSession("S,4,08:30,10:00,B1,202");
EXPECT_EQ(s.day_of_week, 4);
EXPECT_EQ(s.start_time, "08:30");
EXPECT_EQ(s.end_time, "10:00");
}
// tests/preParser_test.cpp
#include <gtest/gtest.h>
#include "parsers/preParser.h"
#include <fstream>
#include <filesystem>

using namespace std;

class PreParserTest : public ::testing::Test {
protected:
    string testCourseDBPath = "test_course_db.txt";
    string testUserInputPath = "../data/userInput.txt";

    void SetUp() override {
        // Write dummy course DB
        ofstream db(testCourseDBPath);
        db << "Software Engineering\n"
           << "42\n"
           << "Dr. Who\n"
           << "L S,3,09:00,11:00,BuildA,Room5\n"
           << "$$$$\n";
        db.close();

        // Write user input
        ofstream user(testUserInputPath);
        user << "42\n";
        user.close();
    }

    void TearDown() override {
        remove(testCourseDBPath.c_str());
        remove(testUserInputPath.c_str());
    }
};

TEST_F(PreParserTest, ValidTime) {
EXPECT_TRUE(isValidTime("13:45"));
EXPECT_FALSE(isValidTime("1345"));
EXPECT_FALSE(isValidTime("24:00"));
EXPECT_FALSE(isValidTime("10:70"));
}

TEST_F(PreParserTest, ParsesValidCourseDB) {
auto courses = parseCourseDB(testCourseDBPath);
ASSERT_EQ(courses.size(), 1);
EXPECT_EQ(courses[0].id, 42);
EXPECT_EQ(courses[0].Lectures.size(), 1);
EXPECT_EQ(courses[0].Lectures[0].day_of_week, 3);
}

TEST(SessionTest, InvalidSessionThrows) {
EXPECT_THROW(parseSingleSession("S,2,12:00,11:00,Bld,Room"), std::invalid_argument);
}

TEST(SessionTest, ValidSessionParsedCorrectly) {
Session s = parseSingleSession("S,4,08:30,10:00,B1,202");
EXPECT_EQ(s.day_of_week, 4);
EXPECT_EQ(s.start_time, "08:30");
EXPECT_EQ(s.end_time, "10:00");
}
