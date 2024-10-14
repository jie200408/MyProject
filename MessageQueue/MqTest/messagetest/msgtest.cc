#include "../../MqServer/message.hpp"
#include <gtest/gtest.h>

mq::MessageManager::ptr mmp;

class MessageTest : public testing::Environment {
public:
    virtual void SetUp() override {
        mmp = std::make_shared<mq::MessageManager>("./data/message/");
        mmp->initQueueMessage("queue1");
    }

    virtual void TearDown() override {
        // mmp->clear();
    }
};

// // 测试插入数据
// TEST(message_test, insert_test) {
//     mq::BasicProperties properties;
//     properties.set_id(mq::UUIDHelper::uuid());
//     properties.set_routing_key("news.music.pop");
//     properties.set_delivery_mode(mq::DeliveryMode::DURABLE);

//     mmp->insert("queue1", &properties, "Hello World-1", mq::DeliveryMode::DURABLE);
//     mmp->insert("queue1", nullptr, "Hello World-2", mq::DeliveryMode::DURABLE);
//     mmp->insert("queue1", nullptr, "Hello World-3", mq::DeliveryMode::DURABLE);
//     mmp->insert("queue1", nullptr, "Hello World-4", mq::DeliveryMode::DURABLE);
//     mmp->insert("queue1", nullptr, "Hello World-5", mq::DeliveryMode::UNDURABLE);

//     ASSERT_EQ(mmp->durable_count("queue1"), 4);
//     ASSERT_EQ(mmp->getable_count("queue1"), 5);
//     ASSERT_EQ(mmp->waitack_count("queue1"), 0);
//     ASSERT_EQ(mmp->total_count("queue1"), 4);
// }


// // 测试拿出数据
// TEST(message_test, select_test) {
//     mq::MessagePtr msgp = mmp->front("queue1");
//     ASSERT_EQ(msgp->payload().body(), "Hello World-1");
//     ASSERT_EQ(mmp->waitack_count("queue1"), 1);
//     ASSERT_EQ(mmp->getable_count("queue1"), 4);

//     msgp = mmp->front("queue1");
//     ASSERT_EQ(msgp->payload().body(), "Hello World-2");
//     ASSERT_EQ(mmp->waitack_count("queue1"), 2);
//     ASSERT_EQ(mmp->getable_count("queue1"), 3);

//     msgp = mmp->front("queue1");
//     ASSERT_EQ(msgp->payload().body(), "Hello World-3");
//     ASSERT_EQ(mmp->waitack_count("queue1"), 3);
//     ASSERT_EQ(mmp->getable_count("queue1"), 2);

//     msgp = mmp->front("queue1");
//     ASSERT_EQ(msgp->payload().body(), "Hello World-4");
//     ASSERT_EQ(mmp->waitack_count("queue1"), 4);
//     ASSERT_EQ(mmp->getable_count("queue1"), 1);

//     msgp = mmp->front("queue1");
//     ASSERT_EQ(msgp->payload().body(), "Hello World-5");
//     ASSERT_EQ(mmp->waitack_count("queue1"), 5);
//     ASSERT_EQ(mmp->getable_count("queue1"), 0);
// }

// // 测试恢复数据
// TEST(message_test, recover_test) {
//     ASSERT_EQ(mmp->durable_count("queue1"), 4);
//     ASSERT_EQ(mmp->total_count("queue1"), 4);
//     mq::MessagePtr msgp = mmp->front("queue1");
//     ASSERT_EQ(msgp->payload().body(), "Hello World-1");
//     ASSERT_EQ(mmp->waitack_count("queue1"), 1);
//     ASSERT_EQ(mmp->getable_count("queue1"), 3);  

//     msgp = mmp->front("queue1");
//     ASSERT_EQ(msgp->payload().body(), "Hello World-2");
//     ASSERT_EQ(mmp->waitack_count("queue1"), 2);
//     ASSERT_EQ(mmp->getable_count("queue1"), 2);

//     msgp = mmp->front("queue1");
//     ASSERT_EQ(msgp->payload().body(), "Hello World-3");
//     ASSERT_EQ(mmp->waitack_count("queue1"), 3);
//     ASSERT_EQ(mmp->getable_count("queue1"), 1);

//     msgp = mmp->front("queue1");
//     ASSERT_EQ(msgp->payload().body(), "Hello World-4");
//     ASSERT_EQ(mmp->waitack_count("queue1"), 4);
//     ASSERT_EQ(mmp->getable_count("queue1"), 0);
// }

// // 测试删除数据，ack数据
// TEST(message_test, ack_test) {
//     // 从中获取数据，并且删除数据
//     mq::MessagePtr msgp = mmp->front("queue1");
//     mmp->ack("queue1", msgp->payload().properties().id());
//     ASSERT_EQ(mmp->waitack_count("queue1"), 0);
//     ASSERT_EQ(mmp->durable_count("queue1"), 3);
//     ASSERT_EQ(mmp->total_count("queue1"), 4);
// }

TEST(message_test, destroy_test) {
    mmp->destoryQueueMessahe("queue1");
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new MessageTest);
    RUN_ALL_TESTS();
    return 0;
}