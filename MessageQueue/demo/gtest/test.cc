#include <iostream>
#include <gtest/gtest.h>

class MyEnvironment : public testing::Environment {
public:
    virtual void SetUp() override {
        std::cout << "单元测试开始" << std::endl;
    }

    virtual void TearDown() override {
        std::cout << "单元测试结束" << std::endl;
    }
};

TEST(MyEnvironment, test1) {
    std::cout << "测试1" << std::endl;
}

TEST(MyEnvironment, test2) {
    std::cout << "测试2" << std::endl;
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new MyEnvironment);
    RUN_ALL_TESTS();
    return 0;
}