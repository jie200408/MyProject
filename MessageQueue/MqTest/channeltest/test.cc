#include "../../MqServer/channel.hpp"

int main() {
    mq::ChannelManager::ptr cmp = std::make_shared<mq::ChannelManager>();
    cmp->openChannel("tag1", mq::VirtualHost::ptr(), mq::ConsumerManager::ptr(), 
        mq::ProtobufCodecPtr(), muduo::net::TcpConnectionPtr(), mq::threadpool::ptr());
    return 0;
}