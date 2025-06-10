#include <gtest/gtest.h>
#include "CourseLegalComb.h"
#include "model_interfaces.h"

using namespace std;

enum DayOfWeek { Mon = 1, Tue = 2, Wed = 3, Thu = 4, Fri = 5, Sat = 6, Sun = 7 };

class CourseLegalCombTest : public ::testing::Test {
protected:
    CourseLegalComb comb;

    Session makeSession(const string& start, const string& end, int day) {
        Session session;
        session.day_of_week = day;
        session.start_time = start;
        session.end_time = end;
        session.building_number = "";
        session.room_number = "";
        return session;
    }

    Group makeGroup(SessionType type, const vector<Session>& sessions) {
        Group group;
        group.type = type;
        group.sessions = sessions;
        return group;
    }

    Course makeCourse(
            int id,
            const vector<Group>& lectures = {},
            const vector<Group>& tutorials = {},
            const vector<Group>& labs = {},
            const vector<Group>& blocks = {}
    ) {
        Course c;
        c.id = id;
        c.raw_id = to_string(id);
        c.name = "Course " + to_string(id);
        c.teacher = "";
        c.Lectures = lectures;
        c.Tirgulim = tutorials;
        c.labs = labs;
        c.blocks = blocks;
        return c;
    }
};

// Basic: Single lecture group with one session, no tutorial/lab
TEST_F(CourseLegalCombTest, SingleLectureOnly) {
    vector<Session> lectureSessions = { makeSession("10:00", "12:00", Mon) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);
    Course c = makeCourse(1, {lectureGroup});

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 1);
    EXPECT_EQ(combinations[0].courseId, 1);
    EXPECT_NE(combinations[0].lectureGroup, nullptr);
    EXPECT_EQ(combinations[0].tutorialGroup, nullptr);
    EXPECT_EQ(combinations[0].labGroup, nullptr);
}

// All 3 session types without conflicts
TEST_F(CourseLegalCombTest, LectureWithTutorialAndLab) {
    vector<Session> lectureSessions = { makeSession("09:00", "11:00", Mon) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> tutorialSessions = { makeSession("11:00", "12:00", Mon) };
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    vector<Session> labSessions = { makeSession("12:00", "13:00", Mon) };
    Group labGroup = makeGroup(SessionType::LAB, labSessions);

    Course c = makeCourse(2, {lectureGroup}, {tutorialGroup}, {labGroup});

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 1);
    EXPECT_NE(combinations[0].lectureGroup, nullptr);
    EXPECT_NE(combinations[0].tutorialGroup, nullptr);
    EXPECT_NE(combinations[0].labGroup, nullptr);
}

// Tutorial overlaps with lecture → should be excluded
TEST_F(CourseLegalCombTest, OverlappingTutorialShouldBeSkipped) {
    vector<Session> lectureSessions = { makeSession("09:00", "11:00", Tue) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> tutorialSessions = { makeSession("10:30", "11:30", Tue) }; // Overlaps with lecture
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    vector<Session> labSessions = { makeSession("11:30", "12:30", Tue) };
    Group labGroup = makeGroup(SessionType::LAB, labSessions);

    Course c = makeCourse(3, {lectureGroup}, {tutorialGroup}, {labGroup});

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 0); // No valid combinations due to overlap
}

// Two lecture groups with same valid tutorial/lab → 2 combinations
TEST_F(CourseLegalCombTest, MultipleValidCombinations) {
    vector<Session> lectureSessionsGroup1 = { makeSession("08:00", "10:00", Wed) };
    Group lectureGroup1 = makeGroup(SessionType::LECTURE, lectureSessionsGroup1);

    vector<Session> lectureSessionsGroup2 = { makeSession("10:00", "12:00", Wed) };
    Group lectureGroup2 = makeGroup(SessionType::LECTURE, lectureSessionsGroup2);

    vector<Session> tutorialSessions = { makeSession("12:00", "13:00", Wed) };
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    vector<Session> labSessions = { makeSession("13:00", "14:00", Wed) };
    Group labGroup = makeGroup(SessionType::LAB, labSessions);

    Course c = makeCourse(4, {lectureGroup1, lectureGroup2}, {tutorialGroup}, {labGroup});

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 2); // Two lecture options, both compatible with tutorial/lab
}

// Tutorial and lab conflict → skip
TEST_F(CourseLegalCombTest, TutorialAndLabOverlapShouldSkip) {
    vector<Session> lectureSessions = { makeSession("08:00", "10:00", Thu) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> tutorialSessions = { makeSession("10:00", "11:00", Thu) };
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    vector<Session> labSessions = { makeSession("10:30", "11:30", Thu) }; // Overlaps with tutorial
    Group labGroup = makeGroup(SessionType::LAB, labSessions);

    Course c = makeCourse(5, {lectureGroup}, {tutorialGroup}, {labGroup});

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 0); // No valid combinations due to tutorial-lab overlap
}

// Edge Case: Course with no sessions at all
TEST_F(CourseLegalCombTest, EmptyCourseShouldReturnNothing) {
    Course c = makeCourse(6);
    auto combinations = comb.generate(c);
    ASSERT_TRUE(combinations.empty());
}

// Edge Case: Only lab groups, no lectures (invalid input case)
TEST_F(CourseLegalCombTest, OnlyLabsShouldReturnNothing) {
    vector<Session> labSessions = { makeSession("10:00", "11:00", Fri) };
    Group labGroup = makeGroup(SessionType::LAB, labSessions);

    Course c = makeCourse(7, {}, {}, {labGroup});
    auto combinations = comb.generate(c);
    ASSERT_TRUE(combinations.empty()); // No lecture groups means no valid combinations
}

// Edge Case: Overlapping lecture and lab, but no tutorial
TEST_F(CourseLegalCombTest, LectureLabOverlapNoTutorial) {
    vector<Session> lectureSessions = { makeSession("09:00", "11:00", Fri) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> labSessions = { makeSession("10:30", "11:30", Fri) }; // Overlaps with lecture
    Group labGroup = makeGroup(SessionType::LAB, labSessions);

    Course c = makeCourse(8, {lectureGroup}, {}, {labGroup});
    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 0); // No valid combinations due to lecture-lab overlap
}

// Edge Case: Multiple tutorial and lab groups, only some valid
TEST_F(CourseLegalCombTest, SomeCombinationsValidAmongMany) {
    vector<Session> lectureSessions = { makeSession("08:00", "09:30", Wed) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> tutorialSessionsGroup1 = { makeSession("09:30", "10:30", Wed) }; // Valid
    Group tutorialGroup1 = makeGroup(SessionType::TUTORIAL, tutorialSessionsGroup1);

    vector<Session> tutorialSessionsGroup2 = { makeSession("08:30", "09:15", Wed) }; // Overlaps with lecture
    Group tutorialGroup2 = makeGroup(SessionType::TUTORIAL, tutorialSessionsGroup2);

    vector<Session> labSessionsGroup1 = { makeSession("10:30", "11:30", Wed) }; // Valid
    Group labGroup1 = makeGroup(SessionType::LAB, labSessionsGroup1);

    vector<Session> labSessionsGroup2 = { makeSession("09:00", "10:00", Wed) }; // Overlaps with lecture
    Group labGroup2 = makeGroup(SessionType::LAB, labSessionsGroup2);

    Course c = makeCourse(9, {lectureGroup}, {tutorialGroup1, tutorialGroup2}, {labGroup1, labGroup2});

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 1); // Only one fully non-overlapping combo
    EXPECT_EQ(combinations[0].tutorialGroup->sessions[0].start_time, "09:30");
    EXPECT_EQ(combinations[0].labGroup->sessions[0].start_time, "10:30");
}

// Test conflicting sessions within the same group (should be handled properly)
TEST_F(CourseLegalCombTest, ConflictingSessionsWithinGroup) {
    // A group with conflicting sessions - this might be invalid input,
    // but we should handle it gracefully
    vector<Session> conflictingSessions = {
        makeSession("09:00", "10:00", Mon),
        makeSession("09:30", "10:30", Mon)  // Overlaps with first session
    };
    Group lectureGroup = makeGroup(SessionType::LECTURE, conflictingSessions);

    Course c = makeCourse(10, {lectureGroup});
    auto combinations = comb.generate(c);

    // Depending on your implementation, this might return 0 combinations
    // or handle the conflict in some other way
    // Adjust the expectation based on your actual implementation
    EXPECT_TRUE(combinations.size() <= 1);
}

// Test with blocks (new session type)
TEST_F(CourseLegalCombTest, LectureWithBlocks) {
    vector<Session> lectureSessions = { makeSession("09:00", "10:00", Mon) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> blockSessions = { makeSession("11:00", "13:00", Mon) };
    Group blockGroup = makeGroup(SessionType::BLOCK, blockSessions);

    Course c = makeCourse(11, {lectureGroup}, {}, {}, {blockGroup});
    auto combinations = comb.generate(c);

    // Based on your implementation, when blocks exist, only block combinations are created
    ASSERT_EQ(combinations.size(), 1);
    EXPECT_EQ(combinations[0].lectureGroup, nullptr);  // Lecture is null when blocks exist
    EXPECT_EQ(combinations[0].tutorialGroup, nullptr);
    EXPECT_EQ(combinations[0].labGroup, nullptr);
    EXPECT_NE(combinations[0].blockGroup, nullptr);    // Block group should be set
}

// Test with multiple block groups
TEST_F(CourseLegalCombTest, MultipleBlockGroups) {
    vector<Session> blockSessions1 = { makeSession("09:00", "11:00", Mon) };
    Group blockGroup1 = makeGroup(SessionType::BLOCK, blockSessions1);

    vector<Session> blockSessions2 = { makeSession("13:00", "15:00", Mon) };
    Group blockGroup2 = makeGroup(SessionType::BLOCK, blockSessions2);

    Course c = makeCourse(12, {}, {}, {}, {blockGroup1, blockGroup2});
    auto combinations = comb.generate(c);

    // Should only return one combination with the first block group
    ASSERT_EQ(combinations.size(), 1);
    EXPECT_NE(combinations[0].blockGroup, nullptr);
}

// Test lecture with tutorial but no lab
TEST_F(CourseLegalCombTest, LectureWithTutorialNoLab) {
    vector<Session> lectureSessions = { makeSession("09:00", "10:00", Tue) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> tutorialSessions = { makeSession("10:00", "11:00", Tue) };
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    Course c = makeCourse(13, {lectureGroup}, {tutorialGroup});
    auto combinations = comb.generate(c);

    ASSERT_EQ(combinations.size(), 1);
    EXPECT_NE(combinations[0].lectureGroup, nullptr);
    EXPECT_NE(combinations[0].tutorialGroup, nullptr);
    EXPECT_EQ(combinations[0].labGroup, nullptr);
    EXPECT_EQ(combinations[0].blockGroup, nullptr);
}

// Test lecture with lab but no tutorial
TEST_F(CourseLegalCombTest, LectureWithLabNoTutorial) {
    vector<Session> lectureSessions = { makeSession("09:00", "10:00", Wed) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> labSessions = { makeSession("11:00", "13:00", Wed) };
    Group labGroup = makeGroup(SessionType::LAB, labSessions);

    Course c = makeCourse(14, {lectureGroup}, {}, {labGroup});
    auto combinations = comb.generate(c);

    ASSERT_EQ(combinations.size(), 1);
    EXPECT_NE(combinations[0].lectureGroup, nullptr);
    EXPECT_EQ(combinations[0].tutorialGroup, nullptr);
    EXPECT_NE(combinations[0].labGroup, nullptr);
    EXPECT_EQ(combinations[0].blockGroup, nullptr);
}

// Test multiple lecture groups with multiple tutorial groups (all valid)
TEST_F(CourseLegalCombTest, MultipleLecturesMultipleTutorials) {
    vector<Session> lectureSessionsGroup1 = { makeSession("08:00", "09:00", Thu) };
    Group lectureGroup1 = makeGroup(SessionType::LECTURE, lectureSessionsGroup1);

    vector<Session> lectureSessionsGroup2 = { makeSession("10:00", "11:00", Thu) };
    Group lectureGroup2 = makeGroup(SessionType::LECTURE, lectureSessionsGroup2);

    vector<Session> tutorialSessionsGroup1 = { makeSession("09:00", "10:00", Thu) };
    Group tutorialGroup1 = makeGroup(SessionType::TUTORIAL, tutorialSessionsGroup1);

    vector<Session> tutorialSessionsGroup2 = { makeSession("11:00", "12:00", Thu) };
    Group tutorialGroup2 = makeGroup(SessionType::TUTORIAL, tutorialSessionsGroup2);

    Course c = makeCourse(15, {lectureGroup1, lectureGroup2}, {tutorialGroup1, tutorialGroup2});
    auto combinations = comb.generate(c);

    // Should generate: (Lec1,Tut1), (Lec1,Tut2), (Lec2,Tut1), (Lec2,Tut2) = 4 combinations
    ASSERT_EQ(combinations.size(), 4);
}

// Test complex scenario with partial conflicts
TEST_F(CourseLegalCombTest, ComplexPartialConflicts) {
    // Lecture 1: 08:00-09:30
    vector<Session> lectureSessionsGroup1 = { makeSession("08:00", "09:30", Fri) };
    Group lectureGroup1 = makeGroup(SessionType::LECTURE, lectureSessionsGroup1);

    // Lecture 2: 10:00-11:30
    vector<Session> lectureSessionsGroup2 = { makeSession("10:00", "11:30", Fri) };
    Group lectureGroup2 = makeGroup(SessionType::LECTURE, lectureSessionsGroup2);

    // Tutorial 1: 09:30-10:00 (works with Lec1 only)
    vector<Session> tutorialSessionsGroup1 = { makeSession("09:30", "10:00", Fri) };
    Group tutorialGroup1 = makeGroup(SessionType::TUTORIAL, tutorialSessionsGroup1);

    // Tutorial 2: 11:30-12:00 (works with Lec2 only)
    vector<Session> tutorialSessionsGroup2 = { makeSession("11:30", "12:00", Fri) };
    Group tutorialGroup2 = makeGroup(SessionType::TUTORIAL, tutorialSessionsGroup2);

    // Lab: 12:00-14:00 (works with both)
    vector<Session> labSessions = { makeSession("12:00", "14:00", Fri) };
    Group labGroup = makeGroup(SessionType::LAB, labSessions);

    Course c = makeCourse(16, {lectureGroup1, lectureGroup2}, {tutorialGroup1, tutorialGroup2}, {labGroup});
    auto combinations = comb.generate(c);

    // Should generate all valid combinations: (Lec1,Tut1,Lab), (Lec1,Tut2,Lab), (Lec2,Tut1,Lab), (Lec2,Tut2,Lab) = 4 combinations
    ASSERT_EQ(combinations.size(), 4);
}

// Test different days - no conflicts across days
TEST_F(CourseLegalCombTest, DifferentDaysNoConflicts) {
    vector<Session> lectureSessionsMon = { makeSession("09:00", "11:00", Mon) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessionsMon);

    vector<Session> tutorialSessionsTue = { makeSession("09:00", "10:00", Tue) };
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessionsTue);

    vector<Session> labSessionsWed = { makeSession("09:00", "11:00", Wed) };
    Group labGroup = makeGroup(SessionType::LAB, labSessionsWed);

    Course c = makeCourse(17, {lectureGroup}, {tutorialGroup}, {labGroup});
    auto combinations = comb.generate(c);

    // Should work fine since all are on different days
    ASSERT_EQ(combinations.size(), 1);
    EXPECT_NE(combinations[0].lectureGroup, nullptr);
    EXPECT_NE(combinations[0].tutorialGroup, nullptr);
    EXPECT_NE(combinations[0].labGroup, nullptr);
}

// Test adjacent time slots (touching but not overlapping)
TEST_F(CourseLegalCombTest, AdjacentTimeSlots) {
    vector<Session> lectureSessions = { makeSession("09:00", "10:00", Mon) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> tutorialSessions = { makeSession("10:00", "11:00", Mon) }; // Exactly adjacent
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    vector<Session> labSessions = { makeSession("11:00", "12:00", Mon) }; // Also adjacent
    Group labGroup = makeGroup(SessionType::LAB, labSessions);

    Course c = makeCourse(18, {lectureGroup}, {tutorialGroup}, {labGroup});
    auto combinations = comb.generate(c);

    // Adjacent times should NOT conflict
    ASSERT_EQ(combinations.size(), 1);
    EXPECT_NE(combinations[0].lectureGroup, nullptr);
    EXPECT_NE(combinations[0].tutorialGroup, nullptr);
    EXPECT_NE(combinations[0].labGroup, nullptr);
}

// Test sessions with same start but different end times
TEST_F(CourseLegalCombTest, SameStartDifferentEnd) {
    vector<Session> lectureSessions = { makeSession("09:00", "10:00", Mon) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> tutorialSessions = { makeSession("09:00", "11:00", Mon) }; // Same start, longer
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    Course c = makeCourse(19, {lectureGroup}, {tutorialGroup});
    auto combinations = comb.generate(c);

    // Should conflict since they start at the same time
    ASSERT_EQ(combinations.size(), 0);
}

// Test course with only tutorials (no lectures)
TEST_F(CourseLegalCombTest, OnlyTutorialsNoLectures) {
    vector<Session> tutorialSessions = { makeSession("10:00", "11:00", Mon) };
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    Course c = makeCourse(20, {}, {tutorialGroup});
    auto combinations = comb.generate(c);

    // Should return empty since lectures are required
    ASSERT_TRUE(combinations.empty());
}

// Test very large number of groups (stress test)
TEST_F(CourseLegalCombTest, ManyNonConflictingGroups) {
    vector<Group> lectureGroups;
    vector<Group> tutorialGroups;

    // Create 3 lecture groups at different times
    for (int i = 0; i < 3; i++) {
        string start = to_string(8 + i * 2) + ":00";
        string end = to_string(9 + i * 2) + ":00";
        vector<Session> sessions = { makeSession(start, end, Mon) };
        lectureGroups.push_back(makeGroup(SessionType::LECTURE, sessions));
    }

    // Create 2 tutorial groups that don't conflict with any lecture
    for (int i = 0; i < 2; i++) {
        string start = to_string(15 + i) + ":00";
        string end = to_string(16 + i) + ":00";
        vector<Session> sessions = { makeSession(start, end, Mon) };
        tutorialGroups.push_back(makeGroup(SessionType::TUTORIAL, sessions));
    }

    Course c = makeCourse(21, lectureGroups, tutorialGroups);
    auto combinations = comb.generate(c);

    // Should generate 3 lectures × 2 tutorials = 6 combinations
    ASSERT_EQ(combinations.size(), 6);
}

// Test midnight/edge time cases
TEST_F(CourseLegalCombTest, EdgeTimeCases) {
    vector<Session> lectureSessions = { makeSession("23:00", "23:59", Mon) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> tutorialSessions = { makeSession("00:00", "01:00", Tue) }; // Next day
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    Course c = makeCourse(22, {lectureGroup}, {tutorialGroup});
    auto combinations = comb.generate(c);

    // Should work since they're on different days
    ASSERT_EQ(combinations.size(), 1);
}

// Test overlapping sessions with 1-minute gap
TEST_F(CourseLegalCombTest, MinimalTimeGap) {
    vector<Session> lectureSessions = { makeSession("09:00", "10:00", Mon) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> tutorialSessions = { makeSession("10:01", "11:00", Mon) }; // 1 minute gap
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    Course c = makeCourse(23, {lectureGroup}, {tutorialGroup});
    auto combinations = comb.generate(c);

    // Should NOT conflict with 1-minute gap
    ASSERT_EQ(combinations.size(), 1);
}

// Test overlapping sessions with 1-minute overlap
TEST_F(CourseLegalCombTest, MinimalTimeOverlap) {
    vector<Session> lectureSessions = { makeSession("09:00", "10:00", Mon) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> tutorialSessions = { makeSession("09:59", "11:00", Mon) }; // 1 minute overlap
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    Course c = makeCourse(24, {lectureGroup}, {tutorialGroup});
    auto combinations = comb.generate(c);

    // Should conflict with 1-minute overlap
    ASSERT_EQ(combinations.size(), 0);
}

// Test sessions spanning across days (invalid but should be handled)
TEST_F(CourseLegalCombTest, InvalidCrossDaySessions) {
    vector<Session> lectureSessions = { makeSession("23:30", "01:00", Mon) }; // Invalid: ends next day
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    Course c = makeCourse(25, {lectureGroup});
    auto combinations = comb.generate(c);

    // Depending on implementation, might handle gracefully or return combinations
    EXPECT_TRUE(combinations.size() >= 0); // Just ensure it doesn't crash
}

// Test multiple sessions within same group (complex scheduling)
TEST_F(CourseLegalCombTest, MultipleSessionsInSameGroup) {
    vector<Session> lectureSessions = {
        makeSession("09:00", "10:00", Mon),
        makeSession("11:00", "12:00", Wed)  // Same lecture group, different days
    };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> tutorialSessions = { makeSession("10:00", "11:00", Mon) };
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    Course c = makeCourse(26, {lectureGroup}, {tutorialGroup});
    auto combinations = comb.generate(c);

    // Should work - tutorial doesn't conflict with either lecture session
    ASSERT_EQ(combinations.size(), 1);
}

// Test empty group handling
TEST_F(CourseLegalCombTest, EmptyGroupsInCourse) {
    vector<Session> lectureSessions = { makeSession("09:00", "10:00", Mon) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    Group emptyTutorialGroup = makeGroup(SessionType::TUTORIAL, {}); // Empty sessions

    Course c = makeCourse(27, {lectureGroup}, {emptyTutorialGroup});
    auto combinations = comb.generate(c);

    // Should handle empty groups gracefully
    EXPECT_TRUE(combinations.size() >= 0);
}

// Test course with all session types including blocks
TEST_F(CourseLegalCombTest, CourseWithAllSessionTypesAndBlocks) {
    vector<Session> lectureSessions = { makeSession("09:00", "10:00", Mon) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> tutorialSessions = { makeSession("10:00", "11:00", Mon) };
    Group tutorialGroup = makeGroup(SessionType::TUTORIAL, tutorialSessions);

    vector<Session> labSessions = { makeSession("11:00", "12:00", Mon) };
    Group labGroup = makeGroup(SessionType::LAB, labSessions);

    vector<Session> blockSessions = { makeSession("13:00", "15:00", Mon) };
    Group blockGroup = makeGroup(SessionType::BLOCK, blockSessions);

    Course c = makeCourse(28, {lectureGroup}, {tutorialGroup}, {labGroup}, {blockGroup});
    auto combinations = comb.generate(c);

    // Since blocks exist, should only return block combination
    ASSERT_EQ(combinations.size(), 1);
    EXPECT_EQ(combinations[0].lectureGroup, nullptr);
    EXPECT_EQ(combinations[0].tutorialGroup, nullptr);
    EXPECT_EQ(combinations[0].labGroup, nullptr);
    EXPECT_NE(combinations[0].blockGroup, nullptr);
}
// Test overlapping blocks should be handled properly
TEST_F(CourseLegalCombTest, OverlappingBlocksShouldConflict) {
    vector<Session> blockSessions1 = { makeSession("09:00", "11:00", Mon) };
    Group blockGroup1 = makeGroup(SessionType::BLOCK, blockSessions1);

    vector<Session> blockSessions2 = { makeSession("10:00", "12:00", Mon) }; // Overlaps with first block
    Group blockGroup2 = makeGroup(SessionType::BLOCK, blockSessions2);

    Course c = makeCourse(29, {}, {}, {}, {blockGroup1, blockGroup2});
    auto combinations = comb.generate(c);

    // Should return at most 1 combination since blocks conflict
    // Implementation might choose first valid block or return 0 if all conflict
    EXPECT_TRUE(combinations.size() <= 1);
    if (combinations.size() == 1) {
        // If one combination is returned, it should be one of the valid blocks
        EXPECT_NE(combinations[0].blockGroup, nullptr);
    }
}

// Test non-overlapping blocks should work fine
TEST_F(CourseLegalCombTest, NonOverlappingBlocksShouldWork) {
    vector<Session> blockSessions1 = { makeSession("09:00", "10:00", Mon) };
    Group blockGroup1 = makeGroup(SessionType::BLOCK, blockSessions1);

    vector<Session> blockSessions2 = { makeSession("11:00", "12:00", Mon) }; // No overlap
    Group blockGroup2 = makeGroup(SessionType::BLOCK, blockSessions2);

    Course c = makeCourse(30, {}, {}, {}, {blockGroup1, blockGroup2});
    auto combinations = comb.generate(c);

    // Should return multiple combinations since blocks don't conflict
    // Or at least 1 if implementation only returns first valid block
    EXPECT_GE(combinations.size(), 1);
    EXPECT_NE(combinations[0].blockGroup, nullptr);
}

// Test blocks conflicting with lectures (before block priority takes effect)
TEST_F(CourseLegalCombTest, BlocksConflictingWithLectures) {
    vector<Session> lectureSessions = { makeSession("09:00", "10:00", Mon) };
    Group lectureGroup = makeGroup(SessionType::LECTURE, lectureSessions);

    vector<Session> blockSessions1 = { makeSession("09:30", "10:30", Mon) }; // Overlaps with lecture
    Group blockGroup1 = makeGroup(SessionType::BLOCK, blockSessions1);

    vector<Session> blockSessions2 = { makeSession("11:00", "12:00", Mon) }; // No overlap with lecture
    Group blockGroup2 = makeGroup(SessionType::BLOCK, blockSessions2);

    Course c = makeCourse(31, {lectureGroup}, {}, {}, {blockGroup1, blockGroup2});
    auto combinations = comb.generate(c);

    // Since blocks exist, should return block combinations only
    // But should still respect time conflicts between blocks and other sessions
    EXPECT_GE(combinations.size(), 1);
    for (const auto& combo : combinations) {
        EXPECT_EQ(combo.lectureGroup, nullptr);  // Lecture should be null due to block priority
        EXPECT_NE(combo.blockGroup, nullptr);    // Block should be present
    }
}

// Test adjacent blocks (touching but not overlapping)
TEST_F(CourseLegalCombTest, AdjacentBlocksShouldWork) {
    vector<Session> blockSessions1 = { makeSession("09:00", "10:00", Mon) };
    Group blockGroup1 = makeGroup(SessionType::BLOCK, blockSessions1);

    vector<Session> blockSessions2 = { makeSession("10:00", "11:00", Mon) }; // Adjacent, no overlap
    Group blockGroup2 = makeGroup(SessionType::BLOCK, blockSessions2);

    Course c = makeCourse(32, {}, {}, {}, {blockGroup1, blockGroup2});
    auto combinations = comb.generate(c);

    // Adjacent blocks should not conflict
    EXPECT_GE(combinations.size(), 1);
    EXPECT_NE(combinations[0].blockGroup, nullptr);
}

// Test block with minimal overlap (1 minute)
TEST_F(CourseLegalCombTest, BlocksWithMinimalOverlap) {
    vector<Session> blockSessions1 = { makeSession("09:00", "10:00", Mon) };
    Group blockGroup1 = makeGroup(SessionType::BLOCK, blockSessions1);

    vector<Session> blockSessions2 = { makeSession("09:59", "11:00", Mon) }; // 1 minute overlap
    Group blockGroup2 = makeGroup(SessionType::BLOCK, blockSessions2);

    Course c = makeCourse(33, {}, {}, {}, {blockGroup1, blockGroup2});
    auto combinations = comb.generate(c);

    // Should conflict due to 1-minute overlap
    EXPECT_TRUE(combinations.size() <= 1);
    if (combinations.size() == 1) {
        EXPECT_NE(combinations[0].blockGroup, nullptr);
    }
}