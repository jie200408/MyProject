#include <iostream>
#include <vector>
#include <string>
#include <gtest/gtest.h>

class MyTest : public testing::Test {
public:
    static void SetUpTestCase() {
        // 可以在这里对整体需要配置的数据进行配置
        std::cout << "总测试开始" << std::endl;
    }

    // 每一个独立的单元测试都会运行这个
    void SetUp() override {
        std::cout << "单元测试开始" << std::endl;
        _vs.push_back("ele1");
        _vs.push_back("ele2");
    }   

    void TearDown() override {
        std::cout << "单元测试结束" << std::endl;
    }   

    static void TearDownTestCase() {
        std::cout << "总测试结束" << std::endl;
    }

    std::vector<std::string> _vs;
};

TEST_F(MyTest, test1) {
    _vs.push_back("hello");
    ASSERT_EQ(_vs.size(), 3);
}

TEST_F(MyTest, test2) {
    ASSERT_EQ(_vs.size(), 2);
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    return 0;
}