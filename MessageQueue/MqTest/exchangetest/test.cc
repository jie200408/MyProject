#include "../../MqServer/exchange.hpp"
#include <gtest/gtest.h>

mq::ExchangeManager::ptr emp;

class ExchangeTest : public testing::Environment {
public:
    virtual void SetUp() override {
        emp = std::make_shared<mq::ExchangeManager>("./a/test.db");
    }

    virtual void TearDown() override {
        // emp->clear();
    }
};

// TEST(exchange_test, insert_test) {
//     std::unordered_map<std::string, std::string> map{{"k1", "v1"}, {"k2", "v2"}};

//     std::unordered_map<std::string, std::string> map2;

//     emp->declareExchange("exchange1", mq::ExchangeType::DIRECT, true, false, map);
//     emp->declareExchange("exchange2", mq::ExchangeType::DIRECT, true, false, map);
//     emp->declareExchange("exchange3", mq::ExchangeType::DIRECT, true, false, map);
//     emp->declareExchange("exchange4", mq::ExchangeType::DIRECT, true, false, map2);
// }

// TEST(exchange_test, select_test) {
//     ASSERT_EQ(emp->size(), 4);
//     mq::Exchange::ptr exp = emp->selectExchange("exchange3");
//     ASSERT_EQ(exp->name, "exchange3");
//     ASSERT_EQ(exp->durable, true); 
//     ASSERT_EQ(exp->auto_delete, false);
//     ASSERT_EQ(exp->getArgs(), "k1=v1&k2=v2&");
//     ASSERT_EQ(exp->type, mq::ExchangeType::DIRECT); 
// }

// TEST(exchange_test, remove_test) {
//     emp->deleteExchange("exchange1");
//     ASSERT_EQ(emp->exists("exchange1"), false);
//     // 将交换机中的数据都打印出来
//     ASSERT_EQ(emp->size(), 3);
// }

TEST(exchange_test, recover_test) {
    ASSERT_EQ(emp->size(), 3);
    mq::Exchange::ptr exp = emp->selectExchange("exchange3");
    ASSERT_EQ(exp->name, "exchange3");
    ASSERT_EQ(exp->durable, true); 
    ASSERT_EQ(exp->auto_delete, false);
    ASSERT_EQ(exp->getArgs(), "k1=v1&k2=v2&");
    ASSERT_EQ(exp->type, mq::ExchangeType::DIRECT);     

    emp->deleteExchange("exchange2");
    ASSERT_EQ(emp->exists("exchange1"), false);
    ASSERT_EQ(emp->exists("exchange2"), false);
    ASSERT_EQ(emp->size(), 2);

}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new ExchangeTest);
    RUN_ALL_TESTS();
    return 0;
}