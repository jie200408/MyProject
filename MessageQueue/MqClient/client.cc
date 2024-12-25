#include "connection.hpp"
#include "../MqCommon/helper.hpp"
#include "load.hpp"
 
int main() {
    mq::AsyncWorker::ptr awp = std::make_shared<mq::AsyncWorker>();
    mq::Connection::ptr conn = std::make_shared<mq::Connection>("127.0.0.1", 8085, awp);
    mq::Channel::ptr channel = conn->openChannel();
 
    mq::LoadHelper load(channel, "");
    std::pair<bool, mq::UserType> isLoad = load.userLoad();

    if (!isLoad.first) {
        conn->closeChannel(channel);
        return 1;
    }
 
    if (isLoad.second == mq::ADMIN)
        load.adminLoop();
    else if (isLoad.second == mq::PUBLISHER)
        load.publisherLoop();
    else
        load.reciverLoop();

    google::protobuf::Map<std::string, std::string> google_tmp1;
    google::protobuf::Map<std::string, std::string> google_tmp2;
    google::protobuf::Map<std::string, std::string> google_tmp3;
 
    // 主题匹配发送
    //channel->declareExchange("exchange1", mq::ExchangeType::TOPIC, true, false, google_tmp1);
    // // 直接匹配发送
    // channel->declareExchange("exchange1", mq::ExchangeType::DIRECT, true, false, google_tmp1);
    // // 广播匹配发送
    // channel->declareExchange("exchange1", mq::ExchangeType::FANOUT, true, false, google_tmp1);
    // channel->declareQueue("queue1", true, false, false, google_tmp2);
    // channel->declareQueue("queue2", true, false, false, google_tmp3);
    // channel->queueBind("exchange1", "queue1", "news.sport.#");
    // channel->queueBind("exchange1", "queue2", "news.music.#");

    // // 循环发送信息
    // for (int i = 0; i < 10; i++) {
    //     mq::BasicProperties bp;
    //     bp.set_id(mq::UUIDHelper::uuid());
    //     bp.set_routing_key("news.music.pop");
    //     bp.set_delivery_mode(mq::DeliveryMode::DURABLE);
    //     channel->basicPublish("exchange1", &bp, "hello world-" + std::to_string(i));
    // }
 
    // mq::BasicProperties bp;
    // bp.set_id(mq::UUIDHelper::uuid());
    // bp.set_routing_key("queue1");
    // bp.set_delivery_mode(mq::DeliveryMode::DURABLE);
    // channel->basicPublish("exchange1", &bp, "hello world-" + std::to_string(10));
 
    // bp.set_routing_key("news.sport.football");
    // channel->basicPublish("exchange1", &bp, "hello world-" + std::to_string(11));

    // // 先使用广播发送运动新闻
    // mq::BasicProperties bp;
    // bp.set_id(mq::UUIDHelper::uuid());  
    // bp.set_routing_key("news.sport.#");
    // bp.set_delivery_mode(mq::DeliveryMode::DURABLE);
    // std::string body;
    // mq::FileHelper filehelper("./sportnews.txt");
    // filehelper.read(body);
    // channel->basicPublish("exchange1", &bp, body);

    // bp.set_id(mq::UUIDHelper::uuid());  
    // bp.set_routing_key("news.music.#");
    // bp.set_delivery_mode(mq::DeliveryMode::DURABLE);
    // std::string body2;
    // mq::FileHelper filehelper2("./musicnews.txt");
    // filehelper2.read(body2);
    // channel->basicPublish("exchange1", &bp, body2);

    conn->closeChannel(channel);
    return 0;
}