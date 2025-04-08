// tests/preParser_test.cpp
#include <gtest/gtest.h>
#include "parsers/preParser.h"
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
    string testCourseDBPath = "../../testData/validUserInput.txt";
    unordered_set<string> input = readSelectedCourseIDs(testCourseDBPath);

    unordered_set<string> expected = {"83112", "83533", "00001"};

    ASSERT_EQ(input.size(), expected.size());

    for (const auto& id : expected) {
        ASSERT_TRUE(input.count(id)) << "Missing course ID: " << id;
    }
}

// Test: valid POST command in user's input
TEST(PreParserTest, ParsesValidCourseDB) {
    string testCourseDBPath = "../../testData/validDB.txt";
    string testUserInputPath = "../../testData/validUserInput.txt";
    auto courses = parseCourseDB(testCourseDBPath, testUserInputPath);
    ASSERT_EQ(courses.size(), 3);
    EXPECT_EQ(courses[0].id, 83112);
    EXPECT_EQ(courses[0].Lectures.size(), 2);
    EXPECT_EQ(courses[0].Lectures[0].day_of_week, 1);
    std::cout << "Parsed day_of_week: " << courses[0].Lectures[0].day_of_week << std::endl;

}

//Test invalid coruse id:id
TEST(PreParserTest, FailsOnInvalidCourseID) {
    string testCourseDBPath = "../../testData/invalidDB_id.txt";
    string testUserInputPath = "../../testData/validUserInput.txt";
    auto courses = parseCourseDB(testCourseDBPath, testUserInputPath);

    // Depending on behavior: expect 0 valid courses parsed
    ASSERT_EQ(courses.size(), 0) << "Invalid course ID should result in no valid courses.";
}
//Test invalid coruse id:string
TEST(PreParserTest, FailsOnNonNumericFields) {
    string testCourseDBPath = "../../testData/invalidDB_string.txt";
    string testUserInputPath = "../../testData/validUserInput.txt";
    auto courses = parseCourseDB(testCourseDBPath, testUserInputPath);

    // Expect parser to reject the malformed course entries
    ASSERT_EQ(courses.size(), 0) << "Non-numeric fields should be rejected.";
}

//Test: invalid user inputId
TEST(PreParserTest, InvalidUserInput_InvalidID) {
    string testPath = "../../testData/invalidUserInput_id.txt";
    unordered_set<string> input = readSelectedCourseIDs(testPath);
    // Assuming invalid IDs should be skipped or result in an empty set
    ASSERT_EQ(input.size(), 4) << "Expected no valid course IDs, but got some.";
}
//Test: invalid user input too many courses
TEST(PreParserTest, RejectsMoreThanSevenTotalCourses) {
    string testPath = "../../testData/userInput_TooManyTotal.txt";
    unordered_set<string> input = readSelectedCourseIDs(testPath);

    EXPECT_TRUE(input.empty()) << "Expected input to be rejected due to more than 7 valid course IDs.";
}



//Test: invalid user input: no input
TEST(PreParserTest, InvalidUserInput_EmptyFile) {
    string testPath = "../../testData/invalidUserInput_none.txt";
    unordered_set<string> input = readSelectedCourseIDs(testPath);
    EXPECT_TRUE(input.empty()) << "Expected empty set from empty file.";
}
//Test: invalid user input: Garbage text
TEST(PreParserTest, InvalidUserInput_NonNumericStrings) {
    string testPath = "../../testData/invalidUserInput_string.txt";
    unordered_set<string> input = readSelectedCourseIDs(testPath);

    for (const auto& id : input) {
        EXPECT_TRUE(all_of(id.begin(), id.end(), ::isdigit))
                            << "Non-numeric course ID detected: " << id;
    }
}
//Test user input test duplicate user input
TEST(PreParserTest, DuplicateUserInputIDs) {
    string testPath = "../../testData/duplicateUserInput.txt";

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

    // Check for duplicate warning in stderr (optional)
    EXPECT_NE(output.find("Warning: Duplicate course ID found in user input: 83112"), string::npos);
    EXPECT_NE(output.find("Warning: Duplicate course ID found in user input: 00001"), string::npos);
}



