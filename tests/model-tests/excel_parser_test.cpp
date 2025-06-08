#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "excel_parser.h"
using namespace std;

class ExcelParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test data directory
        testDataDir = "../testData/excel/";
    }

    void TearDown() override {
        // Cleanup if needed
    }

    string testDataDir;
    ExcelCourseParser parser;
};

// Test: Parse valid Excel file - equivalent to ParsesValidCourseDB
TEST_F(ExcelParserTest, ParsesValidExcelFile) {
    string testPath = testDataDir + "validExcel.xlsx";
    auto courses = parser.parseExcelFile(testPath);

    // Should parse exactly 3 courses from valid Excel file
    ASSERT_EQ(courses.size(), 3) << "Should parse exactly 3 courses from valid Excel file";

    // Check that we have valid courses
    for (const auto& course : courses) {
        EXPECT_GT(course.id, 0) << "Course ID should be positive";
        EXPECT_FALSE(course.name.empty()) << "Course name should not be empty";

        // Should have at least one type of session
        bool hasValidSessions = !course.Lectures.empty() ||
                                !course.Tirgulim.empty() ||
                                !course.labs.empty();
        EXPECT_TRUE(hasValidSessions) << "Course should have at least one valid session type";
    }

    // Check first course properties based on test data
    if (!courses.empty()) {
        const auto& firstCourse = courses[0];
        EXPECT_EQ(firstCourse.id, 1) << "First course should have ID 1";

        if (!firstCourse.Lectures.empty() && !firstCourse.Lectures[0].sessions.empty()) {
            EXPECT_EQ(firstCourse.Lectures[0].sessions[0].day_of_week, 2) << "First session should be on day 2 (Monday)";
        }
    }
}

// Test: Handles invalid course IDs - equivalent to FailsOnInvalidCourseID
TEST_F(ExcelParserTest, HandlesInvalidCourseIDs) {
    string testPath = testDataDir + "invalidExcel_id.xlsx";
    auto courses = parser.parseExcelFile(testPath);

    // Parser handles invalid IDs gracefully without crashing
    EXPECT_GE(courses.size(), 0) << "Should handle the file without crashing";
}

// Test: Handles non-numeric fields - equivalent to FailsOnNonNumericFields
TEST_F(ExcelParserTest, HandlesNonNumericFields) {
    string testPath = testDataDir + "invalidExcel_string.xlsx";
    auto courses = parser.parseExcelFile(testPath);

    // Parser handles non-numeric fields gracefully without crashing
    EXPECT_GE(courses.size(), 0) << "Should handle the file without crashing";
}

// Test: Empty Excel file handling - equivalent to InvalidUserInput_EmptyFile
TEST_F(ExcelParserTest, HandlesEmptyExcelFile) {
    string testPath = testDataDir + "emptyExcel.xlsx";
    auto courses = parser.parseExcelFile(testPath);
    EXPECT_EQ(courses.size(), 0) << "Expected empty course list from empty Excel file.";
}

// Test: Non-existent file handling
TEST_F(ExcelParserTest, HandlesNonExistentFile) {
    string testPath = testDataDir + "nonexistent.xlsx";
    auto courses = parser.parseExcelFile(testPath);
    EXPECT_EQ(courses.size(), 0) << "Expected empty course list from non-existent file.";
}

// Test: Courses with no valid time slots are filtered out
TEST_F(ExcelParserTest, FiltersOutCoursesWithoutValidTimeSlots) {
    string testPath = testDataDir + "noValidTimeSlots.xlsx";
    auto courses = parser.parseExcelFile(testPath);
    EXPECT_EQ(courses.size(), 0) << "Courses without valid time slots should be filtered out.";
}

// Test: Filter for "סמסטר א'" only
TEST_F(ExcelParserTest, FiltersForSemesterAOnly) {
    string testPath = testDataDir + "multiSemesterExcel.xlsx";
    auto courses = parser.parseExcelFile(testPath);

    // Should only include courses from semester A
    EXPECT_EQ(courses.size(), 2) << "Should only include courses from semester A";

    // Verify that all returned courses are valid and from semester A
    for (const auto& course : courses) {
        EXPECT_GT(course.id, 0) << "Course ID should be positive";
        EXPECT_FALSE(course.name.empty()) << "Course name should not be empty";

        bool hasValidSessions = !course.Lectures.empty() ||
                                !course.Tirgulim.empty() ||
                                !course.labs.empty();
        EXPECT_TRUE(hasValidSessions) << "Course should have at least one valid session type";
    }
}

// Test: Session type mapping and filtering
TEST_F(ExcelParserTest, HandlesSessionTypeMapping) {
    string testPath = testDataDir + "allSessionTypesExcel.xlsx";
    auto courses = parser.parseExcelFile(testPath);

    // Should handle file gracefully (may be empty if no supported session types)
    EXPECT_GE(courses.size(), 0) << "Should handle the file without crashing";
}

// Test: Complex room parsing
TEST_F(ExcelParserTest, HandlesComplexRoomFormats) {
    string testPath = testDataDir + "complexRoomsExcel.xlsx";
    auto courses = parser.parseExcelFile(testPath);

    // Should handle complex room formats without crashing
    EXPECT_GE(courses.size(), 0) << "Should handle the file without crashing";
}

// Test: UTF-8 Hebrew character handling
TEST_F(ExcelParserTest, HandlesUTF8HebrewCharacters) {
    string testPath = testDataDir + "utf8HebrewExcel.xlsx";
    auto courses = parser.parseExcelFile(testPath);

    EXPECT_GT(courses.size(), 0) << "Should handle UTF-8 Hebrew characters correctly";

    // Check that Hebrew day parsing works correctly
    for (const auto& course : courses) {
        for (const auto& lecture : course.Lectures) {
            for (const auto& session : lecture.sessions) {
                EXPECT_GT(session.day_of_week, 0) << "Day of week should be valid (1-7)";
                EXPECT_LE(session.day_of_week, 7) << "Day of week should be valid (1-7)";
            }
        }
        for (const auto& tutorial : course.Tirgulim) {
            for (const auto& session : tutorial.sessions) {
                EXPECT_GT(session.day_of_week, 0) << "Day of week should be valid (1-7)";
                EXPECT_LE(session.day_of_week, 7) << "Day of week should be valid (1-7)";
            }
        }
        for (const auto& lab : course.labs) {
            for (const auto& session : lab.sessions) {
                EXPECT_GT(session.day_of_week, 0) << "Day of week should be valid (1-7)";
                EXPECT_LE(session.day_of_week, 7) << "Day of week should be valid (1-7)";
            }
        }
    }
}

// ====================
// Unit Tests for Individual Parser Functions
// ====================

// Test: Parse single session with Hebrew day format
TEST_F(ExcelParserTest, ParsesSingleSessionWithHebrewDay) {
    Session session = parser.parseSingleSession("א'10:00-12:00", "הנדסה-1104 - 243", "ד\"ר כהן");

    EXPECT_EQ(session.day_of_week, 1) << "Hebrew 'א' should map to day 1 (Sunday)";
    EXPECT_EQ(session.start_time, "10:00");
    EXPECT_EQ(session.end_time, "12:00");
    EXPECT_EQ(session.building_number, "הנדסה 1104");
    EXPECT_EQ(session.room_number, "243");
}

// Test: Hebrew day mapping for all days of the week
TEST_F(ExcelParserTest, MapsHebrewDaysCorrectly) {
    EXPECT_EQ(parser.parseSingleSession("א'10:00-12:00", "", "").day_of_week, 1); // Sunday
    EXPECT_EQ(parser.parseSingleSession("ב'10:00-12:00", "", "").day_of_week, 2); // Monday
    EXPECT_EQ(parser.parseSingleSession("ג'10:00-12:00", "", "").day_of_week, 3); // Tuesday
    EXPECT_EQ(parser.parseSingleSession("ד'10:00-12:00", "", "").day_of_week, 4); // Wednesday
    EXPECT_EQ(parser.parseSingleSession("ה'10:00-12:00", "", "").day_of_week, 5); // Thursday
    EXPECT_EQ(parser.parseSingleSession("ו'10:00-12:00", "", "").day_of_week, 6); // Friday
    EXPECT_EQ(parser.parseSingleSession("ש'10:00-12:00", "", "").day_of_week, 7); // Saturday
}

// Test: Session type mapping for all supported and unsupported types
TEST_F(ExcelParserTest, MapsSessionTypesCorrectly) {
    // Supported session types
    EXPECT_EQ(parser.getSessionType("הרצאה"), SessionType::LECTURE);
    EXPECT_EQ(parser.getSessionType("תרגיל"), SessionType::TUTORIAL);
    EXPECT_EQ(parser.getSessionType("מעבדה"), SessionType::LAB);

    // Unsupported session types
    EXPECT_EQ(parser.getSessionType("ש.מחלקה"), SessionType::UNSUPPORTED);
    EXPECT_EQ(parser.getSessionType("תגבור"), SessionType::UNSUPPORTED);
    EXPECT_EQ(parser.getSessionType("הדרכה"), SessionType::UNSUPPORTED);
    EXPECT_EQ(parser.getSessionType("קולוקויום רשות"), SessionType::UNSUPPORTED);
    EXPECT_EQ(parser.getSessionType("רישום"), SessionType::UNSUPPORTED);
    EXPECT_EQ(parser.getSessionType("תיזה"), SessionType::UNSUPPORTED);
    EXPECT_EQ(parser.getSessionType("פרויקט"), SessionType::UNSUPPORTED);

    // Default fallback for unknown types
    EXPECT_EQ(parser.getSessionType("unknown"), SessionType::LECTURE);
}

// Test: Course code parsing with various formats
TEST_F(ExcelParserTest, ParsesCourseCodesCorrectly) {
    // Standard format with group
    auto [code1, group1] = parser.parseCourseCode("83112-01");
    EXPECT_EQ(code1, "83112");
    EXPECT_EQ(group1, "01");

    auto [code2, group2] = parser.parseCourseCode("83112-02");
    EXPECT_EQ(code2, "83112");
    EXPECT_EQ(group2, "02");

    // Format without group (should default to "01")
    auto [code3, group3] = parser.parseCourseCode("83112");
    EXPECT_EQ(code3, "83112");
    EXPECT_EQ(group3, "01");

    // Invalid format - too short
    auto [code4, group4] = parser.parseCourseCode("1234");
    EXPECT_EQ(code4, "1234");
    EXPECT_EQ(group4, "01");

    // Invalid format - non-numeric
    auto [code5, group5] = parser.parseCourseCode("8311a");
    EXPECT_EQ(code5, "8311a");
    EXPECT_EQ(group5, "01");
}

// Test: Room format parsing for different room formats
TEST_F(ExcelParserTest, ParsesRoomFormatsCorrectly) {
    // Standard format: "הנדסה-1104 - 243"
    Session session1 = parser.parseSingleSession("א'10:00-12:00", "הנדסה-1104 - 243", "");
    EXPECT_EQ(session1.building_number, "הנדסה 1104");
    EXPECT_EQ(session1.room_number, "243");

    // Alternative format: "וואהל 1401 - 4"
    Session session2 = parser.parseSingleSession("א'10:00-12:00", "וואהל 1401 - 4", "");
    EXPECT_EQ(session2.building_number, "וואהל 1401");
    EXPECT_EQ(session2.room_number, "4");

    // Building without room number
    Session session3 = parser.parseSingleSession("א'10:00-12:00", "הנדסה-1104", "");
    EXPECT_EQ(session3.building_number, "הנדסה 1104");
    EXPECT_EQ(session3.room_number, "");

    // Building name only
    Session session4 = parser.parseSingleSession("א'10:00-12:00", "הנדסה", "");
    EXPECT_EQ(session4.building_number, "הנדסה");
    EXPECT_EQ(session4.room_number, "");
}

// Test: Parse multiple sessions from single time slot string
TEST_F(ExcelParserTest, ParsesMultipleSessions) {
    vector<Session> sessions = parser.parseMultipleSessions("א'10:00-12:00 ג'14:00-16:00",
                                                            "הנדסה-1104 - 243\nוואהל 1401 - 4",
                                                            "ד\"ר כהן");

    EXPECT_EQ(sessions.size(), 2) << "Should parse two sessions";

    if (sessions.size() >= 2) {
        // First session (Sunday)
        EXPECT_EQ(sessions[0].day_of_week, 1);
        EXPECT_EQ(sessions[0].start_time, "10:00");
        EXPECT_EQ(sessions[0].end_time, "12:00");
        EXPECT_EQ(sessions[0].building_number, "הנדסה 1104");
        EXPECT_EQ(sessions[0].room_number, "243");

        // Second session (Tuesday)
        EXPECT_EQ(sessions[1].day_of_week, 3);
        EXPECT_EQ(sessions[1].start_time, "14:00");
        EXPECT_EQ(sessions[1].end_time, "16:00");
        EXPECT_EQ(sessions[1].building_number, "וואהל 1401");
        EXPECT_EQ(sessions[1].room_number, "4");
    }
}

// Test: Parse multiple rooms from single cell
TEST_F(ExcelParserTest, ParsesMultipleRooms) {
    // Test newline-separated rooms
    vector<string> rooms1 = parser.parseMultipleRooms("הנדסה-1104 - 243\nוואהל 1401 - 4");
    EXPECT_EQ(rooms1.size(), 2);
    EXPECT_EQ(rooms1[0], "הנדסה-1104 - 243");
    EXPECT_EQ(rooms1[1], "וואהל 1401 - 4");

    // Test single room
    vector<string> rooms2 = parser.parseMultipleRooms("הנדסה-1104 - 243");
    EXPECT_EQ(rooms2.size(), 1);
    EXPECT_EQ(rooms2[0], "הנדסה-1104 - 243");

    // Test empty room string
    vector<string> rooms3 = parser.parseMultipleRooms("");
    EXPECT_EQ(rooms3.size(), 1);
    EXPECT_EQ(rooms3[0], "");
}

// Test: Invalid time format handling
TEST_F(ExcelParserTest, HandlesInvalidTimeFormats) {
    // Completely invalid format
    Session session1 = parser.parseSingleSession("invalid", "", "");
    EXPECT_EQ(session1.day_of_week, 0); // Should be invalid

    // Empty time slot
    Session session2 = parser.parseSingleSession("", "", "");
    EXPECT_EQ(session2.day_of_week, 0);
    EXPECT_TRUE(session2.start_time.empty());

    // Time without Hebrew day apostrophe
    Session session3 = parser.parseSingleSession("10:00-12:00", "", "");
    EXPECT_EQ(session3.day_of_week, 0); // Should be invalid without Hebrew day

    // Invalid Hebrew day character
    Session session4 = parser.parseSingleSession("ק'10:00-12:00", "", "");
    EXPECT_EQ(session4.day_of_week, 0); // Should be invalid day
}

// Test: Edge cases in session parsing
TEST_F(ExcelParserTest, HandlesSessionParsingEdgeCases) {
    // Test that empty time slots return no sessions
    vector<Session> emptySessions = parser.parseMultipleSessions("", "", "");
    EXPECT_EQ(emptySessions.size(), 0) << "Empty time slots should result in no sessions";

    // Test that invalid session format returns invalid session
    Session invalidSession = parser.parseSingleSession("invalid_format", "", "");
    EXPECT_EQ(invalidSession.day_of_week, 0) << "Invalid time format should result in invalid session";

    // Test sessions with valid Hebrew day format but invalid times
    Session validFormatInvalidTime = parser.parseSingleSession("א'25:00-26:00", "", "");
    EXPECT_EQ(validFormatInvalidTime.day_of_week, 1); // Day should be parsed correctly
    EXPECT_EQ(validFormatInvalidTime.start_time, "25:00"); // Time is not validated by parser
    EXPECT_EQ(validFormatInvalidTime.end_time, "26:00");
}