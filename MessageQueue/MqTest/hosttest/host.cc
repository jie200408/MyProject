#include <gtest/gtest.h>
#include "../../MqServer/host.hpp"
#include "../../MqCommon/msg.pb.h"

class HostTest : public testing::Test {
public:
    // 每一个独立的单元测试都会运行这个
    void SetUp() override {
        _hp = std::make_shared<mq::VirtualHost>("host1", "./data/host1/message", "./data/host1/host1.db");
        auto empty_map = google::protobuf::Map<std::string, std::string>();
        _hp->declareExchange("exchange1", mq::ExchangeType::DIRECT, true, false, empty_map);
        _hp->declareExchange("exchange2", mq::ExchangeType::DIRECT, true, false, empty_map);
        _hp->declareExchange("exchange3", mq::ExchangeType::DIRECT, true, false, empty_map);
    
        _hp->declareQueue("queue1", true, false, false, empty_map);
        _hp->declareQueue("queue2", true, false, false, empty_map);
        _hp->declareQueue("queue3", true, false, false, empty_map);

        _hp->bind("exchange1", "queue1", "news.music.pop");
        _hp->bind("exchange1", "queue2", "news.music.pop");
        _hp->bind("exchange1", "queue3", "news.music.pop");

        _hp->bind("exchange2", "queue1", "news.music.pop");
        _hp->bind("exchange2", "queue2", "news.music.pop");
        _hp->bind("exchange2", "queue3", "news.music.pop");

        _hp->bind("exchange3", "queue1", "news.music.pop");
        _hp->bind("exchange3", "queue2", "news.music.pop");
        _hp->bind("exchange3", "queue3", "news.music.pop");

        _hp->basicPublish("queue1", nullptr, "Hello World-1");
        _hp->basicPublish("queue1", nullptr, "Hello World-2");
        _hp->basicPublish("queue1", nullptr, "Hello World-3");
    
        _hp->basicPublish("queue2", nullptr, "Hello World-1");
        _hp->basicPublish("queue2", nullptr, "Hello World-2");
        _hp->basicPublish("queue2", nullptr, "Hello World-3");
    
        _hp->basicPublish("queue3", nullptr, "Hello World-1");
        _hp->basicPublish("queue3", nullptr, "Hello World-2");
        _hp->basicPublish("queue3", nullptr, "Hello World-3");
    }   

    void TearDown() override {
        _hp->clear();
    }   

    mq::VirtualHost::ptr _hp;
};

// 测试插入
TEST_F(HostTest, insert_test) {
    ASSERT_EQ(_hp->existsExchange("exchange1"), true);
    ASSERT_EQ(_hp->existsExchange("exchange2"), true);
    ASSERT_EQ(_hp->existsExchange("exchange3"), true);

    ASSERT_EQ(_hp->existsQueue("queue1"), true);
    ASSERT_EQ(_hp->existsQueue("queue2"), true);
    ASSERT_EQ(_hp->existsQueue("queue3"), true);

    ASSERT_EQ(_hp->existsBinding("exchange1", "queue1"), true);
    ASSERT_EQ(_hp->existsBinding("exchange1", "queue2"), true);
    ASSERT_EQ(_hp->existsBinding("exchange1", "queue3"), true);

    ASSERT_EQ(_hp->existsBinding("exchange2", "queue1"), true);
    ASSERT_EQ(_hp->existsBinding("exchange2", "queue2"), true);
    ASSERT_EQ(_hp->existsBinding("exchange2", "queue3"), true); 

    ASSERT_EQ(_hp->existsBinding("exchange3", "queue1"), true);
    ASSERT_EQ(_hp->existsBinding("exchange3", "queue2"), true);
    ASSERT_EQ(_hp->existsBinding("exchange3", "queue3"), true);

}

TEST_F(HostTest, recive_test) {
    mq::MessagePtr mp = _hp->basicConsume("queue1");
    ASSERT_EQ(mp->payload().body(), "Hello World-1");

    mp = _hp->basicConsume("queue1");
    ASSERT_EQ(mp->payload().body(), "Hello World-2");

    mp = _hp->basicConsume("queue1");
    ASSERT_EQ(mp->payload().body(), "Hello World-3");

    mp = _hp->basicConsume("queue1");
    ASSERT_EQ(mp.get(), nullptr);
}

// 测试删除单元
TEST_F(HostTest, delete_test) {
    _hp->deleteExchange("exchange1");
    ASSERT_EQ(_hp->existsExchange("exchange1"), false);
    ASSERT_EQ(_hp->existsBinding("exchange1", "queue1"), false);
    ASSERT_EQ(_hp->existsBinding("exchange1", "queue2"), false);
    ASSERT_EQ(_hp->existsBinding("exchange1", "queue3"), false);    

    _hp->deleteQueue("queue1");
    ASSERT_EQ(_hp->existsQueue("queue1"), false);
    ASSERT_EQ(_hp->existsBinding("exchange1", "queue1"), false);
    ASSERT_EQ(_hp->existsBinding("exchange2", "queue1"), false);
    ASSERT_EQ(_hp->existsBinding("exchange3", "queue1"), false);

    mq::MessagePtr mp = _hp->basicConsume("queue1");
    ASSERT_EQ(mp.get(), nullptr);    

    ASSERT_EQ(_hp->existsBinding("exchange2", "queue2"), true);
    _hp->unBind("exchange2", "queue2");
    ASSERT_EQ(_hp->existsBinding("exchange2", "queue2"), false);
}

TEST_F(HostTest, ack_test) {
    mq::MessagePtr mp = _hp->basicConsume("queue1");
    _hp->basicAck("queue1", mp->payload().properties().id());

    mp = _hp->basicConsume("queue1");
    _hp->basicAck("queue1", mp->payload().properties().id());

    mp = _hp->basicConsume("queue1");
    _hp->basicAck("queue1", mp->payload().properties().id());
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    return 0;
}