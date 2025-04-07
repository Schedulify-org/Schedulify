#include <gtest/gtest.h>
#include <fstream>
#include <cstdio> // for std::remove
#include "../include/parsers/preParser.h"

// Utility: Create temp file
void writeToFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    file << content;
    file.close();
}

// Clean up temp file
void removeFile(const std::string& filename) {
    std::remove(filename.c_str());
}

TEST(ReadUserInputIDs, ValidFile) {
    const std::string file = "test_userInput.txt";
    writeToFile(file, "123 456\n789 012\n");

    auto result = readUserInputIDsFromFile(file);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], std::make_pair("123", "456"));
    EXPECT_EQ(result[1], std::make_pair("789", "012"));

    removeFile(file);
}

TEST(ReadUserInputIDs, InvalidLineFormat) {
    const std::string file = "test_userInput_invalid.txt";
    writeToFile(file, "invalid_line_without_space\n");

    auto result = readUserInputIDsFromFile(file);
    EXPECT_TRUE(result.empty());

    removeFile(file);
}

TEST(ParseSingleSession, ValidInput) {
    std::string sessionStr = "S,3,10:00,12:00,B1,101";
    Session s = parseSingleSession(sessionStr);

    EXPECT_EQ(s.day_of_week, 3);
    EXPECT_EQ(s.start_time, "10:00");
    EXPECT_EQ(s.end_time, "12:00");
    EXPECT_EQ(s.building_number, "B1");
    EXPECT_EQ(s.room_number, "101");
}

TEST(ParseSingleSession, InvalidDayOfWeek) {
    std::string sessionStr = "S,8,10:00,12:00,B1,101";
    EXPECT_THROW(parseSingleSession(sessionStr), std::invalid_argument);
}

TEST(ParseMultipleSessions, MultipleValidSessions) {
    std::string line = "L S,2,09:00,10:00,B2,202 S,4,11:00,12:00,B3,303";
    auto sessions = parseMultipleSessions(line);

    ASSERT_EQ(sessions.size(), 2);
    EXPECT_EQ(sessions[0].day_of_week, 2);
    EXPECT_EQ(sessions[1].day_of_week, 4);
}

TEST(ParseMultipleSessions, SomeMalformedSessions) {
    std::string line = "T S,1,08:00,09:00,B1,101 S,bad_data S,5,13:00,14:00,B2,202";
    auto sessions = parseMultipleSessions(line);

    ASSERT_EQ(sessions.size(), 2);  // One should be skipped
    EXPECT_EQ(sessions[0].day_of_week, 1);
    EXPECT_EQ(sessions[1].day_of_week, 5);
}

// Optional: parseCourseDB test - basic stub
TEST(ParseCourseDB, NoFileFound) {
    auto courses = parseCourseDB("nonexistent_file.txt");
    EXPECT_TRUE(courses.empty());
}
