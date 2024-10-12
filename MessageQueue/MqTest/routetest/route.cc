#include "../../MqServer/route.hpp"
#include <gtest/gtest.h>

class RouteTest : public testing::Environment {
public:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};

TEST(route_test, legal_test) {
    std::string rkeys1("new.music.pop");
    std::string rkeys2("new.music_..pop");
    std::string rkeys3("new.music.,pop");
    std::string rkeys4("new.music. .pop");
    std::string rkeys5("new.music12356.pop");
    std::string rkeys6("new.music>.pop");

    ASSERT_EQ(mq::Router::isLegalRoutingKey(rkeys1), true);
    ASSERT_EQ(mq::Router::isLegalRoutingKey(rkeys2), true);
    ASSERT_EQ(mq::Router::isLegalRoutingKey(rkeys3), false);
    ASSERT_EQ(mq::Router::isLegalRoutingKey(rkeys4), false);
    ASSERT_EQ(mq::Router::isLegalRoutingKey(rkeys5), true);
    ASSERT_EQ(mq::Router::isLegalRoutingKey(rkeys6), false);

    std::string bkey1 = "news.music.pop";
    std::string bkey2 = "news.#.music.pop";
    std::string bkey3 = "news.#.*.music.pop";//
    std::string bkey4 = "news.*.#.music.pop";//
    std::string bkey5 = "news.#.#.music.pop";//
    std::string bkey6 = "news.*.*.music.pop";
    std::string bkey7 = "news.,music_123.pop";//

    ASSERT_EQ(mq::Router::isLegalBindingKey(bkey1), true);
    ASSERT_EQ(mq::Router::isLegalBindingKey(bkey2), true);
    ASSERT_EQ(mq::Router::isLegalBindingKey(bkey3), false);
    ASSERT_EQ(mq::Router::isLegalBindingKey(bkey4), false);
    ASSERT_EQ(mq::Router::isLegalBindingKey(bkey5), false);
    ASSERT_EQ(mq::Router::isLegalBindingKey(bkey6), true);
    ASSERT_EQ(mq::Router::isLegalBindingKey(bkey7), false);
}

TEST(route_test, route) {
    std::vector<std::string> bkeys = {
        "aaa",
        "aaa.bbb",
        "aaa.bbb", 
        "aaa.bbb",
        "aaa.#.bbb",
        "aaa.bbb.#",
        "#.bbb.ccc",
        "aaa.bbb.ccc",
        "aaa.*",
        "aaa.*.bbb",
        "*.aaa.bbb", 
        "#",   
        "aaa.#", 
        "aaa.#",  
        "aaa.#.ccc",
        "aaa.#.ccc",
        "aaa.#.ccc",
        "#.ccc",
        "#.ccc",
        "aaa.#.ccc.ccc",
        "aaa.#.bbb.*.bbb"
    };
    std::vector<std::string> rkeys = {
        "aaa",
        "aaa.bbb",    
        "aaa.bbb.ccc",        
        "aaa.ccc",        
        "aaa.bbb.ccc",        
        "aaa.ccc.bbb",        
        "aaa.bbb.ccc.ddd",    
        "aaa.bbb.ccc",       
        "aaa.bbb",         
        "aaa.bbb.ccc",      
        "aaa.bbb",         
        "aaa.bbb.ccc",       
        "aaa.bbb",        
        "aaa.bbb.ccc",     
        "aaa.ccc",        
        "aaa.bbb.ccc",       
        "aaa.aaa.bbb.ccc",  
        "ccc",         
        "aaa.bbb.ccc",    
        "aaa.bbb.ccc.ccc.ccc",
        "aaa.ddd.ccc.bbb.eee.bbb"
    };
    std::vector<bool> result = {
        true,
        true,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        false,
        false,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    };
    for (int i = 0; i < bkeys.size(); i++) {
        ASSERT_EQ(mq::Router::route(mq::ExchangeType::TOPIC, rkeys[i], bkeys[i]), result[i]);
    }
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new RouteTest);
    RUN_ALL_TESTS();
    return 0;
}