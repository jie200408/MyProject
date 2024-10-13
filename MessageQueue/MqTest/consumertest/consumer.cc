#include "../../MqServer/consumer.hpp"
#include <gtest/gtest.h>

mq::ConsumerManager::ptr cmp;

class ConsumerTest : public testing::Environment {
public:
    virtual void SetUp() override {
        cmp = std::make_shared<mq::ConsumerManager>();
        cmp->initQueueConsumer("queue1");
    }

    virtual void TearDown() override {
        cmp->clear();
    }
};

void cb(const std::string& q, const mq::BasicProperties* bp, const std::string b) {

}

TEST(consumer_test, insert_test) {
    cmp->create("consumer1", "queue1", false, cb);
    cmp->create("consumer2", "queue1", false, cb);
    cmp->create("consumer3", "queue1", false, cb);

    ASSERT_EQ(cmp->exists("queue1", "consumer1"), true);
    ASSERT_EQ(cmp->exists("queue1", "consumer2"), true);
    ASSERT_EQ(cmp->exists("queue1", "consumer3"), true);
}

TEST(consumer_test, remove_test) {
    cmp->remove("queue1", "consumer1");
    ASSERT_EQ(cmp->exists("queue1", "consumer1"), false);
}

TEST(consumer_test, choose_test) {
    mq::Consumer::ptr cp = cmp->choose("queue1");
    ASSERT_NE(cp.get(), nullptr);
    ASSERT_EQ(cp->tag, "consumer2");

    cp = cmp->choose("queue1");
    ASSERT_NE(cp.get(), nullptr);
    ASSERT_EQ(cp->tag, "consumer3");

    cp = cmp->choose("queue1");
    ASSERT_NE(cp.get(), nullptr);
    ASSERT_EQ(cp->tag, "consumer2");
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new ConsumerTest);
    RUN_ALL_TESTS();
    return 0;
}


