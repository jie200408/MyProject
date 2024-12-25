#include "connection.hpp"
 
// 收到消息之后的回调函数
void cb(const mq::Channel::ptr& channel, const std::string& consumer_tag, const mq::BasicProperties* bp, const std::string& body) {
    std::cout << consumer_tag << " 得到消息: " << body << std::endl;
    channel->basicAck(bp->id());
}

int main(int argc ,char* argv[]) {
    if (argc != 2) {
        DLOG("please input the two args: ./consume_client queue1\n");
        return -1;
    }
    // 创建连接以及信道
    mq::AsyncWorker::ptr awp = std::make_shared<mq::AsyncWorker>();
    mq::Connection::ptr conn = std::make_shared<mq::Connection>("127.0.0.1", 8085, awp);    
    mq::Channel::ptr channel = conn->openChannel();
    
 
    // 主题匹配接收
    google::protobuf::Map<std::string, std::string> google_tmp;
 
    // // 直接匹配接收
    // channel->declareExchange("exchange1", mq::ExchangeType::DIRECT, true, false, google_tmp);
    // // 广播匹配接收
    channel->declareExchange("exchange1", mq::ExchangeType::FANOUT, true, false, google_tmp);
 
    // channel->declareExchange("exchange1", mq::ExchangeType::TOPIC, true, false, google_tmp);
    channel->declareQueue("queue1", true, false, false, google_tmp);
    channel->declareQueue("queue2", true, false, false, google_tmp);
    channel->queueBind("exchange1", "queue1", "queue1");
    channel->queueBind("exchange1", "queue2", "news.music.#");
 
    
    auto callback = std::bind(cb, channel, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    
 
    channel->basicConsume("consumer1", argv[1], false, callback);
 
    // 主线程在这循环等待
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(3));
 
    conn->closeChannel(channel);
 
}