//#include <gtest/gtest.h>
//
//// Optional: readable days
//enum DayOfWeek {
//    Mon = 0, Tue, Wed, Thu, Fri, Sat, Sun
//};
//
//class ScheduleTest : public ::testing::Test {
//protected:
//    Session *createSession(const std::string &start, const std::string &end, int day) {
//        return new Session{day, start, end, "", ""};
//    }
//
//    void TearDown() override {
//        for (auto *ptr: cleanup)
//            delete ptr;
//        cleanup.clear();
//    }
//
//    std::vector<const Session *> cleanup;
//};
//
//TEST_F(ScheduleTest, TimeToMinutesConversion) {
//    EXPECT_EQ(timeToMinutes("00:00"), 0);
//    EXPECT_EQ(timeToMinutes("01:30"), 90);
//    EXPECT_EQ(timeToMinutes("12:00"), 720);
//    EXPECT_EQ(timeToMinutes("23:59"), 1439);
//}
//
//TEST_F(ScheduleTest, SessionsDoNotOverlapDifferentDays) {
//    auto *s1 = createSession("10:00", "11:00", Mon);
//    auto *s2 = createSession("10:00", "11:00", Tue);
//    cleanup.push_back(s1);
//    cleanup.push_back(s2);
//
//    EXPECT_FALSE(isOverlap(s1, s2));
//}
//
//TEST_F(ScheduleTest, SessionsOverlapSameDay) {
//    auto *s1 = createSession("10:00", "12:00", Mon);
//    auto *s2 = createSession("11:00", "13:00", Mon);
//    cleanup.push_back(s1);
//    cleanup.push_back(s2);
//
//    EXPECT_TRUE(isOverlap(s1, s2));
//}
//
//TEST_F(ScheduleTest, GetValidCourseCombinations_LectureOnly) {
//    auto *lec = createSession("08:00", "09:00", Wed);
//    std::vector<const Session *> lectures = {lec};
//
//    auto combos = getValidCourseCombinations(101, lectures, {}, {});
//    ASSERT_EQ(combos.size(), 1);
//    EXPECT_EQ(combos[0].lecture, lec);
//    EXPECT_EQ(combos[0].tutorial, nullptr);
//    EXPECT_EQ(combos[0].lab, nullptr);
//}
//
//TEST_F(ScheduleTest, GetValidCourseCombinations_LectureAndTutorial_NoOverlap) {
//    auto *lec = createSession("08:00", "09:00", Wed);
//    auto *tut = createSession("09:00", "10:00", Wed);
//    std::vector<const Session *> lectures = {lec};
//    std::vector<const Session *> tutorials = {tut};
//
//    auto combos = getValidCourseCombinations(102, lectures, tutorials, {});
//    ASSERT_EQ(combos.size(), 1);
//    EXPECT_EQ(combos[0].lecture, lec);
//    EXPECT_EQ(combos[0].tutorial, tut);
//}
//
//TEST_F(ScheduleTest, GetValidCourseCombinations_OverlapSkipped) {
//    auto *lec = createSession("08:00", "10:00", Wed);
//    auto *tut = createSession("09:00", "10:30", Wed); // overlaps
//    std::vector<const Session *> lectures = {lec};
//    std::vector<const Session *> tutorials = {tut};
//
//    auto combos = getValidCourseCombinations(103, lectures, tutorials, {});
//    EXPECT_EQ(combos.size(), 0); // No valid combinations
//}
//
//TEST_F(ScheduleTest, NoConflictWithNonOverlappingSessions) {
//    CourseSelection cs1 = {1, createSession("08:00", "09:00", Mon), nullptr, nullptr};
//    CourseSelection cs2 = {2, createSession("09:00", "10:00", Mon), nullptr, nullptr};
//    cleanup.push_back(cs1.lecture);
//    cleanup.push_back(cs2.lecture);
//
//    std::vector<CourseSelection> current = {cs1};
//    EXPECT_TRUE(noConflict(current, cs2));
//}
//
//TEST_F(ScheduleTest, ConflictWithOverlappingSessions) {
//    CourseSelection cs1 = {1, createSession("08:00", "10:00", Mon), nullptr, nullptr};
//    CourseSelection cs2 = {2, createSession("09:30", "11:00", Mon), nullptr, nullptr};
//    cleanup.push_back(cs1.lecture);
//    cleanup.push_back(cs2.lecture);
//
//    std::vector<CourseSelection> current = {cs1};
//    EXPECT_FALSE(noConflict(current, cs2));
//}
//
//TEST_F(ScheduleTest, BacktrackGeneratesAllValidSchedules) {
//    auto *lec1 = createSession("08:00", "09:00", Mon);
//    auto *lec2 = createSession("09:00", "10:00", Mon);
//    auto *lec3 = createSession("10:00", "11:00", Mon);
//
//    std::vector<std::vector<CourseSelection>> options = {
//            {
//                    {1, lec1, nullptr, nullptr},
//                    {1, lec2, nullptr, nullptr},
//                    {1, lec3, nullptr, nullptr}
//            }
//    };
//
//    std::vector<CourseSelection> current;
//    allPossibleSchedules.clear();
//
//    backtrack(0, current, options);
//    EXPECT_EQ(allPossibleSchedules.size(), 3);
//}
