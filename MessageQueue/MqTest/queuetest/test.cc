#include "../../MqServer/queue.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include <memory>

mq::MsgQueueManager::ptr mqptr = std::make_shared<mq::MsgQueueManager>("./a/test.db");

class MsgQueueTest : public testing::Environment {
public:
    virtual void SetUp() override {
        ILOG("begin to test\n");
    }

    virtual void TearDown() override {
        mqptr->clear();
    }
};

TEST(queue_test, insert_test) {
    std::unordered_map<std::string, std::string> map1{{"k1", "v1"}};
    std::unordered_map<std::string, std::string> map2;

    mqptr->declareQueue("queue1", true, false, false, map1);
    mqptr->declareQueue("queue2", true, false, false, map1);
    mqptr->declareQueue("queue3", true, false, false, map1);
    mqptr->declareQueue("queue4", true, false, false, map1);
}

TEST(queue_test, select_test) {
    ASSERT_EQ(mqptr->exists("queue1"), true);
    ASSERT_EQ(mqptr->exists("queue2"), true);
    ASSERT_EQ(mqptr->exists("queue3"), true);
    ASSERT_EQ(mqptr->exists("queue4"), true);
    ASSERT_EQ(mqptr->size(), 4);
    mq::MsgQueue::ptr mp = mqptr->selectQueue("queue1");
    ASSERT_EQ(mp->name, "queue1");
    ASSERT_EQ(mp->durable, true);
    ASSERT_EQ(mp->exclusive, false);
    ASSERT_EQ(mp->auto_delete, false);
    ASSERT_EQ(mp->getArgs(), "k1=v1&");
}

TEST(queue_test, remove_test) {
    mqptr->deleteQueue("queue3");
    ASSERT_EQ(mqptr->size(), 3);
    ASSERT_EQ(mqptr->exists("queue3"), false);
    
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new MsgQueueTest);
    RUN_ALL_TESTS();
    return 0;
}
