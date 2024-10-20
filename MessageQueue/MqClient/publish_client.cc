#include "connection.hpp"

int main() {
    mq::AsyncWorker::ptr awp = std::make_shared<mq::AsyncWorker>();
    mq::Connection::ptr conn = std::make_shared<mq::Connection>("127.0.0.1", 8085, awp);
    mq::Channel::ptr channel = conn->openChannel();


    google::protobuf::Map<std::string, std::string> google_tmp1;
    google::protobuf::Map<std::string, std::string> google_tmp2;
    google::protobuf::Map<std::string, std::string> google_tmp3;
    channel->declareExchange("exchange1", mq::ExchangeType::TOPIC, true, false, google_tmp1);
    channel->declareQueue("queue1", true, false, false, google_tmp2);
    channel->declareQueue("queue2", true, false, false, google_tmp3);
    channel->queueBind("exchange1", "queue1", "queue1");
    channel->queueBind("exchange1", "queue2", "news.music.#");


    for (int i = 0; i < 10; i++) {
        mq::BasicProperties bp;
        bp.set_id(mq::UUIDHelper::uuid());
        bp.set_routing_key("news.music.pop");
        bp.set_delivery_mode(mq::DeliveryMode::DURABLE);
        channel->basicPublish("exchange1", &bp, "hello world-" + std::to_string(i));
    }

    mq::BasicProperties bp;
    bp.set_id(mq::UUIDHelper::uuid());
    bp.set_routing_key("news.music.classic");
    bp.set_delivery_mode(mq::DeliveryMode::DURABLE);
    channel->basicPublish("exchange1", &bp, "hello world-" + std::to_string(10));

    bp.set_routing_key("news.sport.football");
    channel->basicPublish("exchange1", &bp, "hello world-" + std::to_string(11));

    conn->closeChannel(channel);
    return 0;
}