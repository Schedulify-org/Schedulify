#include "schedule_algorithm/TimeUtils.h"
#include "gtest/gtest.h"
#include "test_helpers.h"
#include "parsers/parseCoursesToVector.h"

using namespace std;

// --- TEST CASES ---

// Test that valid time strings are correctly converted to minutes since 00:00
TEST(TimeUtilsTest, ToMinutes_ValidTimes) {
    EXPECT_EQ(TimeUtils::toMinutes("00:00"), 0);     // Midnight
    EXPECT_EQ(TimeUtils::toMinutes("01:00"), 60);    // One hour
    EXPECT_EQ(TimeUtils::toMinutes("12:30"), 750);   // Half past noon
    EXPECT_EQ(TimeUtils::toMinutes("23:59"), 1439);  // One minute before midnight
}

// Test overlap when sessions are on the same day and partially overlap
TEST(TimeUtilsTest, IsOverlap_SameDayOverlapping) {
    auto s1 = makeSession(1, "10:00", "12:00");  // Session 1: 10:00 - 12:00
    auto s2 = makeSession(1, "11:00", "13:00");  // Session 2: 11:00 - 13:00 (overlaps 11:00 - 12:00)

    EXPECT_TRUE(TimeUtils::isOverlap(&s1, &s2));  // Expect overlap
}

// Test edge case where one session ends exactly when the other begins
TEST(TimeUtilsTest, IsOverlap_SameDayTouching_NotOverlapping) {
    auto s1 = makeSession(1, "10:00", "11:00");  // Ends at 11:00
    auto s2 = makeSession(1, "11:00", "12:00");  // Starts at 11:00

    EXPECT_FALSE(TimeUtils::isOverlap(&s1, &s2));  // Should not be considered overlap
}

// Test that sessions on different days never overlap
TEST(TimeUtilsTest, IsOverlap_DifferentDays) {
    auto s1 = makeSession(1, "09:00", "10:00");  // Monday
    auto s2 = makeSession(2, "09:00", "10:00");  // Tuesday

    EXPECT_FALSE(TimeUtils::isOverlap(&s1, &s2));  // Different days → no overlap
}

// Test that if one session is fully contained within the time range of another, it's considered overlapping
TEST(TimeUtilsTest, IsOverlap_FullyContained) {
    auto s1 = makeSession(3, "10:00", "14:00");  // Outer session
    auto s2 = makeSession(3, "11:00", "13:00");  // Inner session fully inside

    EXPECT_TRUE(TimeUtils::isOverlap(&s1, &s2));  // Full containment is still overlap
}


// Session where end_time < start_time (invalid time range) → still processed numerically
TEST(TimeUtilsTest, IsOverlap_ReversedTimeRange) {
    auto s1 = makeSession(1, "12:00", "10:00");  // Invalid time span
    auto s2 = makeSession(1, "09:00", "11:00");

    EXPECT_FALSE(TimeUtils::isOverlap(&s1, &s2));  // No overlap, treated as invalid
}

// Invalid time string format in toMinutes() → should throw or cause error if not handled
TEST(TimeUtilsTest, ToMinutes_InvalidFormat) {
    EXPECT_ANY_THROW(TimeUtils::toMinutes("invalid"));     // Not a time
    EXPECT_ANY_THROW(TimeUtils::toMinutes("25:61"));       // Invalid hour/minute
    EXPECT_ANY_THROW(TimeUtils::toMinutes("10"));          // Missing minutes
}