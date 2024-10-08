#include "../../MqServer/binding.hpp"
#include <gtest/gtest.h>

mq::BindingManager::ptr bmp;

class BindingsTest : public testing::Environment {
public:
    virtual void SetUp() override {
        bmp = std::make_shared<mq::BindingManager>("./a/test.db");
    }

    virtual void TearDown() override {
        // bmp->clear();
    }
};

// TEST(binding_test, insert_test) {
//     bmp->bind("exchange1", "queue1", "news.sport.football", true);
//     bmp->bind("exchange1", "queue2", "news.gossip.#", true);
//     bmp->bind("exchange1", "queue3", "news.music.classic", true);

//     bmp->bind("exchange2", "queue1", "news.sport.football", true);
//     bmp->bind("exchange2", "queue2", "news.gossip.#", true);
//     bmp->bind("exchange2", "queue3", "news.music.classic", true);

//     ASSERT_EQ(bmp->size(), 6);
// }

// TEST(binding_test, select_test) {
//     mq::Binding::ptr bp = bmp->getBinding("exchange1", "queue2");
//     ASSERT_EQ(bp->exchange_name, "exchange1");
//     ASSERT_EQ(bp->msgqueue_name, "queue2");
//     ASSERT_EQ(bp->binding_key, "news.gossip.#");
// }

// TEST(binding_test, select_exchange_test) {
//     mq::MsgQueueBindingMap mqbm = bmp->getExchangeBindings("exchange1");
//     ASSERT_EQ(mqbm.size(), 3);
//     ASSERT_NE(mqbm.find("queue1"), mqbm.end());
//     ASSERT_NE(mqbm.find("queue2"), mqbm.end());
//     ASSERT_NE(mqbm.find("queue3"), mqbm.end());
// }

// TEST(binding_test, remove_exchange_test) {
//     bmp->removeExchangeBindings("exchange1");
//     ASSERT_EQ(bmp->size(), 3);
//     ASSERT_EQ(bmp->exists("exchange1", "queue1"), false);
//     ASSERT_EQ(bmp->exists("exchange1", "queue2"), false);
//     ASSERT_EQ(bmp->exists("exchange1", "queue3"), false);
    
// }

// TEST(binding_test, remove_queue_test) {
//     bmp->removeMsgQueueBindings("queue1");
//     bmp->removeMsgQueueBindings("queue2");
//     ASSERT_EQ(bmp->exists("exchange2", "queue1"), false);
//     ASSERT_EQ(bmp->exists("exchange2", "queue2"), false);
// }

TEST(binding_test, recover_test) {
    ASSERT_EQ(bmp->size(), 1);
    mq::Binding::ptr bp = bmp->getBinding("exchange2", "queue3");
    ASSERT_EQ(bp->binding_key, "news.music.classic");
}



int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new BindingsTest);
    RUN_ALL_TESTS();
    return 0;
}

