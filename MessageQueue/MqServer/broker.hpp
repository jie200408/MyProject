#ifndef __M_BROKER_H__
#define __M_BROKER_H__

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/TcpServer.h>
#include "../MqThird/include/codec.h"
#include "../MqThird/include/dispatcher.h"
#include "../MqCommon/msg.pb.h"
#include "../MqCommon/proto.pb.h"
#include "../MqCommon/threadpool.hpp"
#include "host.hpp"
#include "consumer.hpp"
#include "connection.hpp"

namespace mq {

    class BrokerServer {
    private:
        using MessagePtr = std::shared_ptr<google::protobuf::Message>;;

        void onConnection(const muduo::net::TcpConnectionPtr& conn) {
            if (conn->connected()) 
                LOG_INFO << "新连接建立成功!";
            else 
                LOG_INFO << "连接关闭!";            
        }
        // 默认的处理函数
        void unUnKnownMessage(const muduo::net::TcpConnectionPtr& conn, const MessagePtr& message, muduo::Timestamp) {
            LOG_INFO << "unUnKnownMessage" << message->GetTypeName();
            conn->shutdown(); // 关闭连接
        }

        // 打开信道
        void onOpenChannel(const muduo::net::TcpConnectionPtr& conn, const openChannelRequestPtr& message, muduo::Timestamp) {
        }

        void onCloseChannel(const muduo::net::TcpConnectionPtr& conn, const closeChannelRequestPtr& message, muduo::Timestamp) {
        }

        void onDeclareExchange(const muduo::net::TcpConnectionPtr& conn, const declareExchangeRequestPtr& message, muduo::Timestamp) {
        }

        void onDeleteExchange(const muduo::net::TcpConnectionPtr& conn, const deleteExchangeRequestPtr& message, muduo::Timestamp) {
        }

        void onOpenChannel(const muduo::net::TcpConnectionPtr& conn, const openChannelRequestPtr& message, muduo::Timestamp) {
        }

        void onOpenChannel(const muduo::net::TcpConnectionPtr& conn, const openChannelRequestPtr& message, muduo::Timestamp) {
        }

        void onOpenChannel(const muduo::net::TcpConnectionPtr& conn, const openChannelRequestPtr& message, muduo::Timestamp) {
        }

        void onOpenChannel(const muduo::net::TcpConnectionPtr& conn, const openChannelRequestPtr& message, muduo::Timestamp) {
        }

        void onOpenChannel(const muduo::net::TcpConnectionPtr& conn, const openChannelRequestPtr& message, muduo::Timestamp) {
        }

        void onOpenChannel(const muduo::net::TcpConnectionPtr& conn, const openChannelRequestPtr& message, muduo::Timestamp) {
        }
    
       void onOpenChannel(const muduo::net::TcpConnectionPtr& conn, const openChannelRequestPtr& message, muduo::Timestamp) {
        }
    public:
        BrokerServer(uint16_t port)
            : _server(&_baseloop, muduo::net::InetAddress("0.0.0.0", port), "Server", muduo::net::TcpServer::kReusePort),
            _dispathcher(std::bind(&BrokerServer::unUnKnownMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
            _codec(std::bind(&ProtobufDispatcher::onProtobufMessage, &_dispathcher, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
        {
            // 现在注册业务处理请求函数
            _dispathcher.registerMessageCallback<openChannelRequest>(std::bind(&BrokerServer::onOpenChannel, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

            _server.setMessageCallback(std::bind(&ProtobufCodec::onMessage, &_codec, 
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _server.setConnectionCallback(std::bind(&BrokerServer::onConnection, this, std::placeholders::_1));
        }      

        void start() {
            
            _server.start();
            _baseloop.loop();
        }

    private:
        muduo::net::TcpServer _server;              // 服务器对象
        muduo::net::EventLoop _baseloop;            // 主事件循环器，响应和监听IO事件
        ProtobufDispatcher _dispathcher;            // 请求分发器对象，需要向分发器中的注册处理函数
        ProtobufCodec _codec;                       // protobuf 协议处理器，针对收到的请求数据进行protobuf协议处理 
        VirtualHost::ptr _virtual_host;             // 虚拟机
        ConsumerManager::ptr _consumer_manager;     // 消费者管理句柄
        ConnectionManager::ptr _connection_manager; // 连接管理句柄
        threadpool::ptr _threadpool;                // 线程池管理句柄
    };

}

#endif