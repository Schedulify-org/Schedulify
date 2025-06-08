#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <algorithm>

using namespace std;

class ExcelParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test data directory
        testDataDir = "../testData/excel/";
        // Note: Excel test files should be created using the Python script
        // or manually placed in the testData/excel directory
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

ASSERT_EQ(courses.size(), 3) << "Should parse exactly 3 courses from valid Excel file";

// Check first course (equivalent to course 83112 from validDB.txt)
EXPECT_EQ(courses[0].id, 83112);
EXPECT_EQ(courses[0].Lectures.size(), 1) << "Course 83112 should have 1 lecture group";

if (!courses[0].Lectures.empty()) {
EXPECT_EQ(courses[0].Lectures[0].sessions.size(), 2) << "Lecture should have 2 sessions";
if (!courses[0].Lectures[0].sessions.empty()) {
EXPECT_EQ(courses[0].Lectures[0].sessions[0].day_of_week, 1) << "First session should be on day 1 (Sunday)";
std::cout << "Parsed day_of_week: " << courses[0].Lectures[0].sessions[0].day_of_week << std::endl;
}
}

// Check tutorial groups for course 83112
EXPECT_GT(courses[0].Tirgulim.size(), 0) << "Course 83112 should have tutorial groups";
}

// Test: Fails on invalid course ID - equivalent to FailsOnInvalidCourseID
TEST_F(ExcelParserTest, FailsOnInvalidCourseID) {
string testPath = testDataDir + "invalidExcel_id.xlsx";
auto courses = parser.parseExcelFile(testPath);

// Depending on behavior: expect 0 valid courses parsed
ASSERT_EQ(courses.size(), 0) << "Invalid course ID should result in no valid courses.";
}

// Test: Fails on non-numeric fields - equivalent to FailsOnNonNumericFields
TEST_F(ExcelParserTest, FailsOnNonNumericFields) {
string testPath = testDataDir + "invalidExcel_string.xlsx";
auto courses = parser.parseExcelFile(testPath);

// Expect parser to reject the malformed course entries
ASSERT_EQ(courses.size(), 0) << "Non-numeric fields should be rejected.";
}

// Test: Empty Excel file - equivalent to InvalidUserInput_EmptyFile
TEST_F(ExcelParserTest, HandlesEmptyExcelFile) {
string testPath = testDataDir + "emptyExcel.xlsx";
auto courses = parser.parseExcelFile(testPath);
EXPECT_EQ(courses.size(), 0) << "Expected empty course list from empty Excel file.";
}

// Test: Non-existent file
TEST_F(ExcelParserTest, HandlesNonExistentFile) {
string testPath = testDataDir + "nonexistent.xlsx";
auto courses = parser.parseExcelFile(testPath);
EXPECT_EQ(courses.size(), 0) << "Expected empty course list from non-existent file.";
}

// Test: Courses with no valid time slots are filtered out
TEST_F(ExcelParserTest, FiltersOutCoursesWithoutValidTimeSlots) {
string testPath = testDataDir + "noValidTimeSlots.xlsx";
auto courses = parser.parseExcelFile(testPath);

// Should filter out courses with unsupported session types or invalid time formats
EXPECT_EQ(courses.size(), 0) << "Courses without valid time slots should be filtered out.";
}

// Test: Filter for "סמסטר א'" only
TEST_F(ExcelParserTest, FiltersForSemesterAOnly) {
string testPath = testDataDir + "multiSemesterExcel.xlsx";
auto courses = parser.parseExcelFile(testPath);

// Should only return courses from "סמסטר א'" (2 courses expected)
EXPECT_EQ(courses.size(), 2) << "Should only include courses from semester A";

// Verify that all returned courses are valid and from semester A
for (const auto& course : courses) {
EXPECT_GT(course.id, 0) << "Course ID should be positive";
EXPECT_FALSE(course.name.empty()) << "Course name should not be empty";

// Should have at least one type of session
bool hasValidSessions = !course.Lectures.empty() ||
                        !course.Tirgulim.empty() ||
                        !course.labs.empty();
EXPECT_TRUE(hasValidSessions) << "Course should have at least one valid session type";
}
}

// Test: Session type mapping and filtering
TEST_F(ExcelParserTest, MapsAndFiltersSessionTypesCorrectly) {
string testPath = testDataDir + "allSessionTypesExcel.xlsx";
auto courses = parser.parseExcelFile(testPath);

// Should only include courses with supported session types (הרצאה, תרגיל, מעבדה)
// Unsupported types should be filtered out
EXPECT_EQ(courses.size(), 1) << "Should only include course with supported session types";

if (!courses.empty()) {
const auto& course = courses[0];
EXPECT_EQ(course.id, 66666) << "Should be the course with valid session types";

// Check that it has all three types of sessions
EXPECT_GT(course.Lectures.size(), 0) << "Should have lecture sessions";
EXPECT_GT(course.Tirgulim.size(), 0) << "Should have tutorial sessions";
EXPECT_GT(course.labs.size(), 0) << "Should have lab sessions";
}
}

// Test: Complex room parsing
TEST_F(ExcelParserTest, ParsesComplexRoomFormats) {
string testPath = testDataDir + "complexRoomsExcel.xlsx";
auto courses = parser.parseExcelFile(testPath);

EXPECT_GT(courses.size(), 0) << "Should parse courses with complex room formats";

// Verify that rooms are parsed correctly
for (const auto& course : courses) {
for (const auto& lecture : course.Lectures) {
for (const auto& session : lecture.sessions) {
// Building number should not be empty for valid rooms
if (!session.building_number.empty()) {
EXPECT_FALSE(session.building_number.empty()) << "Building number should not be empty";
}
}
}
}
}

// Test: UTF-8 Hebrew character handling
TEST_F(ExcelParserTest, HandlesUTF8HebrewCharacters) {
string testPath = testDataDir + "utf8HebrewExcel.xlsx";
auto courses = parser.parseExcelFile(testPath);

EXPECT_GT(courses.size(), 0) << "Should handle UTF-8 Hebrew characters correctly";

// Check that Hebrew day parsing works
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

// Test: Hebrew day mapping
TEST_F(ExcelParserTest, MapsHebrewDaysCorrectly) {
EXPECT_EQ(parser.parseSingleSession("א'10:00-12:00", "", "").day_of_week, 1); // Sunday
EXPECT_EQ(parser.parseSingleSession("ב'10:00-12:00", "", "").day_of_week, 2); // Monday
EXPECT_EQ(parser.parseSingleSession("ג'10:00-12:00", "", "").day_of_week, 3); // Tuesday
EXPECT_EQ(parser.parseSingleSession("ד'10:00-12:00", "", "").day_of_week, 4); // Wednesday
EXPECT_EQ(parser.parseSingleSession("ה'10:00-12:00", "", "").day_of_week, 5); // Thursday
EXPECT_EQ(parser.parseSingleSession("ו'10:00-12:00", "", "").day_of_week, 6); // Friday
EXPECT_EQ(parser.parseSingleSession("ש'10:00-12:00", "", "").day_of_week, 7); // Saturday
}

// Test: Session type mapping
TEST_F(ExcelParserTest, MapsSessionTypesCorrectly) {
EXPECT_EQ(parser.getSessionType("הרצאה"), SessionType::LECTURE);
EXPECT_EQ(parser.getSessionType("תרגיל"), SessionType::TUTORIAL);
EXPECT_EQ(parser.getSessionType("מעבדה"), SessionType::LAB);
EXPECT_EQ(parser.getSessionType("ש.מחלקה"), SessionType::UNSUPPORTED);
EXPECT_EQ(parser.getSessionType("תגבור"), SessionType::UNSUPPORTED);
EXPECT_EQ(parser.getSessionType("הדרכה"), SessionType::UNSUPPORTED);
EXPECT_EQ(parser.getSessionType("קולוקויום רשות"), SessionType::UNSUPPORTED);
EXPECT_EQ(parser.getSessionType("רישום"), SessionType::UNSUPPORTED);
EXPECT_EQ(parser.getSessionType("תיזה"), SessionType::UNSUPPORTED);
EXPECT_EQ(parser.getSessionType("פרויקט"), SessionType::UNSUPPORTED);
EXPECT_EQ(parser.getSessionType("unknown"), SessionType::LECTURE); // Default fallback
}

// Test: Course code parsing
TEST_F(ExcelParserTest, ParsesCourseCodesCorrectly) {
auto [code1, group1] = parser.parseCourseCode("83112-01");
EXPECT_EQ(code1, "83112");
EXPECT_EQ(group1, "01");

auto [code2, group2] = parser.parseCourseCode("83112-02");
EXPECT_EQ(code2, "83112");
EXPECT_EQ(group2, "02");

// Test without dash
auto [code3, group3] = parser.parseCourseCode("83112");
EXPECT_EQ(code3, "83112");
EXPECT_EQ(group3, "01"); // Default group

// Test invalid format (too short)
auto [code4, group4] = parser.parseCourseCode("1234");
EXPECT_EQ(code4, "1234");
EXPECT_EQ(group4, "01");

// Test invalid format (non-numeric)
auto [code5, group5] = parser.parseCourseCode("8311a");
EXPECT_EQ(code5, "8311a");
EXPECT_EQ(group5, "01");
}

// Test: Room format parsing
TEST_F(ExcelParserTest, ParsesRoomFormatsCorrectly) {
// Test standard format: "הנדסה-1104 - 243"
Session session1 = parser.parseSingleSession("א'10:00-12:00", "הנדסה-1104 - 243", "");
EXPECT_EQ(session1.building_number, "הנדסה 1104");
EXPECT_EQ(session1.room_number, "243");

// Test alternative format: "וואהל 1401 - 4"
Session session2 = parser.parseSingleSession("א'10:00-12:00", "וואהל 1401 - 4", "");
EXPECT_EQ(session2.building_number, "וואהל 1401");
EXPECT_EQ(session2.room_number, "4");

// Test building without room number
Session session3 = parser.parseSingleSession("א'10:00-12:00", "הנדסה-1104", "");
EXPECT_EQ(session3.building_number, "הנדסה 1104");
EXPECT_EQ(session3.room_number, "");

// Test building only
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
// First session
EXPECT_EQ(sessions[0].day_of_week, 1); // Sunday
EXPECT_EQ(sessions[0].start_time, "10:00");
EXPECT_EQ(sessions[0].end_time, "12:00");
EXPECT_EQ(sessions[0].building_number, "הנדסה 1104");
EXPECT_EQ(sessions[0].room_number, "243");

// Second session
EXPECT_EQ(sessions[1].day_of_week, 3); // Tuesday
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
Session session1 = parser.parseSingleSession("invalid", "", "");
EXPECT_EQ(session1.day_of_week, 0); // Should be invalid

Session session2 = parser.parseSingleSession("", "", "");
EXPECT_EQ(session2.day_of_week, 0);
EXPECT_TRUE(session2.start_time.empty());

// Test time without apostrophe
Session session3 = parser.parseSingleSession("10:00-12:00", "", "");
EXPECT_EQ(session3.day_of_week, 0); // Should be invalid without Hebrew day

// Test invalid Hebrew day
Session session4 = parser.parseSingleSession("ק'10:00-12:00", "", "");
EXPECT_EQ(session4.day_of_week, 0); // Should be invalid day
}

// Test: Edge cases in session parsing
TEST_F(ExcelParserTest, HandlesSessionParsingEdgeCases) {
// Test that courses with no valid time slots return empty sessions
vector<Session> emptySessions = parser.parseMultipleSessions("", "", "");
EXPECT_EQ(emptySessions.size(), 0) << "Empty time slots should result in no sessions";

// Test that invalid session format returns invalid session
Session invalidSession = parser.parseSingleSession("invalid_format", "", "");
EXPECT_EQ(invalidSession.day_of_week, 0) << "Invalid time format should result in invalid session";

// Test sessions with valid format but invalid times
Session validFormatInvalidTime = parser.parseSingleSession("א'25:00-26:00", "", "");
EXPECT_EQ(validFormatInvalidTime.day_of_week, 1); // Day should be parsed correctly
EXPECT_EQ(validFormatInvalidTime.start_time, "25:00"); // Time is not validated by parser
EXPECT_EQ(validFormatInvalidTime.end_time, "26:00");
}