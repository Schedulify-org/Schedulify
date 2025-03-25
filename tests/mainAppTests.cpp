#include "../include/main_app.h"
#include <gtest/gtest.h>

// Test: valid POST command in user's input
TEST(MainTest, TestOfTest) {
    int num = getNumForTest();
    bool validate = (num == 5);

    EXPECT_TRUE(validate);
}