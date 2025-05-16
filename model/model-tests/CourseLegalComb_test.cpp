#include <gtest/gtest.h>
#include "schedule_algorithm/CourseLegalComb.h"

enum DayOfWeek { Mon = 0, Tue, Wed, Thu, Fri, Sat, Sun };

class CourseLegalCombTest : public ::testing::Test {
protected:
    CourseLegalComb comb;

    Session makeSession(const std::string& start, const std::string& end, int day) {
        return Session{day, start, end, "", ""};
    }

    Course makeCourse(
            int id,
            const std::vector<Session>& lectures,
            const std::vector<Session>& tutorials = {},
            const std::vector<Session>& labs = {}
    ) {
        Course c;
        c.id = id;
        c.Lectures = lectures;
        c.Tirgulim = tutorials;
        c.labs = labs;
        return c;
    }
};

// Basic: Single lecture, no tutorial/lab
TEST_F(CourseLegalCombTest, SingleLectureOnly) {
    Course c = makeCourse(1, { makeSession("10:00", "12:00", Mon) });

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 1);
    EXPECT_EQ(combinations[0].lecture->start_time, "10:00");
    EXPECT_EQ(combinations[0].tutorial, nullptr);
    EXPECT_EQ(combinations[0].lab, nullptr);
}

// All 3 session types without conflicts
TEST_F(CourseLegalCombTest, LectureWithTutorialAndLab) {
    Course c = makeCourse(
            2,
            { makeSession("09:00", "11:00", Mon) },
            { makeSession("11:00", "12:00", Mon) },
            { makeSession("12:00", "13:00", Mon) }
    );

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 1);
    EXPECT_NE(combinations[0].tutorial, nullptr);
    EXPECT_NE(combinations[0].lab, nullptr);
}

// Tutorial overlaps with lecture → should be excluded
TEST_F(CourseLegalCombTest, OverlappingTutorialShouldBeSkipped) {
    Course c = makeCourse(
            3,
            { makeSession("09:00", "11:00", Tue) },
            { makeSession("10:30", "11:30", Tue) },
            { makeSession("11:30", "12:30", Tue) }
    );

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 0);
}

// Two lectures with same valid tutorial/lab → 2 combinations
TEST_F(CourseLegalCombTest, MultipleValidCombinations) {
    Course c = makeCourse(
            4,
            {
                    makeSession("08:00", "10:00", Wed),
                    makeSession("10:00", "12:00", Wed)
            },
            { makeSession("12:00", "13:00", Wed) },
            { makeSession("13:00", "14:00", Wed) }
    );

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 2);
}

// Tutorial and lab conflict → skip
TEST_F(CourseLegalCombTest, TutorialAndLabOverlapShouldSkip) {
    Course c = makeCourse(
            5,
            { makeSession("08:00", "10:00", Thu) },
            { makeSession("10:00", "11:00", Thu) },
            { makeSession("10:30", "11:30", Thu) }
    );

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 0);
}

// Edge Case: Course with no sessions at all
TEST_F(CourseLegalCombTest, EmptyCourseShouldReturnNothing) {
    Course c = makeCourse(6, {});
    auto combinations = comb.generate(c);
    ASSERT_TRUE(combinations.empty());
}

// Edge Case: Only lab sessions, no lectures (invalid input case)
TEST_F(CourseLegalCombTest, OnlyLabsShouldReturnNothing) {
    Course c = makeCourse(7, {}, {}, { makeSession("10:00", "11:00", Fri) });
    auto combinations = comb.generate(c);
    ASSERT_TRUE(combinations.empty());
}

// Edge Case: Overlapping lecture and lab, but no tutorial
TEST_F(CourseLegalCombTest, LectureLabOverlapNoTutorial) {
    Course c = makeCourse(
            8,
            { makeSession("09:00", "11:00", Fri) },
            {},  // no tutorial
            { makeSession("10:30", "11:30", Fri) }  // overlaps with lecture
    );
    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 0);
}

// Edge Case: Multiple tutorials and labs, only some valid
TEST_F(CourseLegalCombTest, SomeCombinationsValidAmongMany) {
    Course c = makeCourse(
            9,
            { makeSession("08:00", "09:30", Wed) },
            {
                    makeSession("09:30", "10:30", Wed),  // valid
                    makeSession("08:30", "09:15", Wed)   // overlaps
            },
            {
                    makeSession("10:30", "11:30", Wed),  // valid
                    makeSession("09:00", "10:00", Wed)   // overlaps
            }
    );

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 1);  // Only one fully non-overlapping combo
    EXPECT_EQ(combinations[0].tutorial->start_time, "09:30");
    EXPECT_EQ(combinations[0].lab->start_time, "10:30");
}
