#include <gtest/gtest.h>
#include "../include/CourseLegalComb.h"
#include "../include/TimeUtils.h"

// Optional: Enum for readable day values
enum DayOfWeek { Mon = 0, Tue, Wed, Thu, Fri, Sat, Sun };

class CourseLegalCombTest : public ::testing::Test {
protected:
    CourseLegalComb comb;

    Session makeSession(const string& start, const string& end, int day) {
        return Session{day, start, end, "", ""};
    }

    Course makeCourse(
            int id,
            const vector<Session>& lectures,
            const vector<Session>& tutorials = {},
            const vector<Session>& labs = {}
    ) {
        Course c;
        c.id = id;
        c.Lectures = lectures;
        c.Tirgulim = tutorials;
        c.labs = labs;
        return c;
    }
};

TEST_F(CourseLegalCombTest, SingleLectureOnly) {
    Course c = makeCourse(1, { makeSession("10:00", "12:00", Mon) });

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 1);
    EXPECT_EQ(combinations[0].lecture->start_time, "10:00");
    EXPECT_EQ(combinations[0].tutorial, nullptr);
    EXPECT_EQ(combinations[0].lab, nullptr);
}

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

TEST_F(CourseLegalCombTest, OverlappingTutorialShouldBeSkipped) {
    Course c = makeCourse(
            3,
            { makeSession("09:00", "11:00", Tue) },
            { makeSession("10:30", "11:30", Tue) }, // overlaps with lecture
            { makeSession("11:30", "12:30", Tue) }
    );

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 0); // Tutorial overlaps, no valid combination
}

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
    // 2 lectures × 1 tutorial × 1 lab = 2 valid combinations
    ASSERT_EQ(combinations.size(), 2);
}

TEST_F(CourseLegalCombTest, TutorialAndLabOverlapShouldSkip) {
    Course c = makeCourse(
            5,
            { makeSession("08:00", "10:00", Thu) },
            { makeSession("10:00", "11:00", Thu) },
            { makeSession("10:30", "11:30", Thu) } // overlaps with tutorial
    );

    auto combinations = comb.generate(c);
    ASSERT_EQ(combinations.size(), 0); // Lab conflicts with tutorial
}
