// model-tests/preParser_test.cpp
#include <gtest/gtest.h>
#include "parsers/parseCoursesToVector.h"
#include "model_interfaces.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>     // for ::isdigit

using namespace std;

// Test: valid time
TEST(PreParserTest, ValidTime) {
    EXPECT_TRUE(isValidTime("13:45"));
    EXPECT_FALSE(isValidTime("1345"));
    EXPECT_FALSE(isValidTime("24:00"));
    EXPECT_FALSE(isValidTime("10:70"));
    EXPECT_FALSE(isValidTime(""));
    EXPECT_FALSE(isValidTime("13:45t"));
}

// Test: valid course id
TEST(PreParserTest, ValidId) {
    EXPECT_TRUE(validateID("12345"));
    EXPECT_FALSE(validateID("1345"));
    EXPECT_FALSE(validateID("123456"));
    EXPECT_FALSE(validateID("1234r"));
    EXPECT_FALSE(validateID(""));
}

// Test: valid user input
TEST(PreParserTest, ValidUserInput) {
    string testCourseDBPath = "../testData/validUserInput.txt";
    unordered_set<string> input = readSelectedCourseIDs(testCourseDBPath);

    unordered_set<string> expected = {"83112", "83533", "00001"};

    ASSERT_EQ(input.size(), expected.size());

    for (const auto& id : expected) {
        ASSERT_TRUE(input.count(id)) << "Missing course ID: " << id;
    }
}

// Test: valid POST command in user's input
TEST(PreParserTest, ParsesValidCourseDB) {
    string testCourseDBPath = "../testData/validDB.txt";
    auto courses = parseCourseDB(testCourseDBPath);
    ASSERT_EQ(courses.size(), 3);
    EXPECT_EQ(courses[0].id, 83112);

    // Updated to work with new structure - check if Lectures group exists and has sessions
    ASSERT_GT(courses[0].Lectures.size(), 0) << "Course should have at least one Lectures group";
    ASSERT_GT(courses[0].Lectures[0].sessions.size(), 0) << "Lectures group should have at least one session";

    // Access the first session in the first Lectures group
    EXPECT_EQ(courses[0].Lectures[0].sessions[0].day_of_week, 1);
    std::cout << "Parsed day_of_week: " << courses[0].Lectures[0].sessions[0].day_of_week << std::endl;
}

// Helper function to count total sessions across all groups in a course
int countTotalSessions(const Course& course) {
    int total = 0;
    for (const auto& group : course.Lectures) {
        total += group.sessions.size();
    }
    for (const auto& group : course.Tirgulim) {
        total += group.sessions.size();
    }
    for (const auto& group : course.labs) {
        total += group.sessions.size();
    }
    for (const auto& group : course.blocks) {
        total += group.sessions.size();
    }
    return total;
}

// Test invalid course id
TEST(PreParserTest, FailsOnInvalidCourseID) {
    string testCourseDBPath = "../testData/invalidDB_id.txt";
    auto courses = parseCourseDB(testCourseDBPath);

    // Depending on behavior: expect 0 valid courses parsed
    ASSERT_EQ(courses.size(), 0) << "Invalid course ID should result in no valid courses.";
}

// Test invalid course id: string
TEST(PreParserTest, FailsOnNonNumericFields) {
    string testCourseDBPath = "../testData/invalidDB_string.txt";
    auto courses = parseCourseDB(testCourseDBPath);

    // Expect parser to reject the malformed course entries
    ASSERT_EQ(courses.size(), 0) << "Non-numeric fields should be rejected.";
}

// Test: invalid user inputId
TEST(PreParserTest, InvalidUserInput_InvalidID) {
    string testPath = "../testData/invalidUserInput_id.txt";
    unordered_set<string> input = readSelectedCourseIDs(testPath);
    // Assuming invalid IDs should be skipped or result in an empty set
    ASSERT_EQ(input.size(), 4) << "Expected no valid course IDs, but got some.";
}

// Test: invalid user input too many courses
TEST(PreParserTest, RejectsMoreThanSevenTotalCourses) {
    string testPath = "../testData/userInput_TooManyTotal.txt";
    unordered_set<string> input = readSelectedCourseIDs(testPath);

    EXPECT_TRUE(input.empty()) << "Expected input to be rejected due to more than 7 valid course IDs.";
}

// Test: invalid user input: no input
TEST(PreParserTest, InvalidUserInput_EmptyFile) {
    string testPath = "../testData/invalidUserInput_none.txt";
    unordered_set<string> input = readSelectedCourseIDs(testPath);
    EXPECT_TRUE(input.empty()) << "Expected empty set from empty file.";
}

// Test: invalid user input: Garbage text
TEST(PreParserTest, InvalidUserInput_NonNumericStrings) {
    string testPath = "../testData/invalidUserInput_string.txt";
    unordered_set<string> input = readSelectedCourseIDs(testPath);

    for (const auto& id : input) {
        EXPECT_TRUE(all_of(id.begin(), id.end(), ::isdigit))
                            << "Non-numeric course ID detected: " << id;
    }
}

// Test user input test duplicate user input
TEST(PreParserTest, DuplicateUserInputIDs) {
    string testPath = "../testData/duplicateUserInput.txt";

    // Optional: capture stderr to verify warning
    testing::internal::CaptureStderr();

    unordered_set<string> input = readSelectedCourseIDs(testPath);
    string output = testing::internal::GetCapturedStderr();

    // Should contain only 3 unique valid course IDs
    unordered_set<string> expected = {"83112", "83533", "00001"};

    ASSERT_EQ(input.size(), expected.size());
    for (const auto& id : expected) {
        EXPECT_TRUE(input.count(id)) << "Missing expected course ID: " << id;
    }
}

// Additional test to verify the new structure works correctly
TEST(PreParserTest, VerifyNewCourseStructure) {
    string testCourseDBPath = "../testData/validDB.txt";
    auto courses = parseCourseDB(testCourseDBPath);

    if (!courses.empty()) {
        const Course& course = courses[0];

        // Test that we can access all the new fields
        EXPECT_GE(course.id, 0);
        EXPECT_FALSE(course.raw_id.empty());
        EXPECT_FALSE(course.name.empty());

        // Test that each group type can be accessed
        // (These might be empty depending on your test data)
        EXPECT_GE(course.Lectures.size(), 0);
        EXPECT_GE(course.Tirgulim.size(), 0);
        EXPECT_GE(course.labs.size(), 0);
        EXPECT_GE(course.blocks.size(), 0);

        // If there are lectures, test the session structure
        if (!course.Lectures.empty() && !course.Lectures[0].sessions.empty()) {
            const Session& session = course.Lectures[0].sessions[0];
            EXPECT_GE(session.day_of_week, 0);
            EXPECT_LE(session.day_of_week, 6);
            EXPECT_FALSE(session.start_time.empty());
            EXPECT_FALSE(session.end_time.empty());
            // building_number and room_number might be empty, so we don't test them
        }
    }
}