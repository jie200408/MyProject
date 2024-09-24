#include <iostream>
#include <gtest/gtest.h>

TEST(my_test, great_test) {
    int age = 88;
    EXPECT_GT(age, 83);
    std::cout << "ok" << std::endl;   
}

TEST(my_test, less_test) {
    int age = 88;
    ASSERT_LT(age, 83);
    std::cout << "ok" << std::endl;   
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    return 0;
}