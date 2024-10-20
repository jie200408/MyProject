#ifndef __M_CONNECTION_H__
#define __M_CONNECTION_H__

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/base/CountDownLatch.h>
#include <iostream>
#include "../MqThird/include/codec.h"
#include "../MqThird/include/dispatcher.h"
#include "channel.hpp"
#include "worker.hpp"

namespace mq {

    class Connection {
    private:
        // 对于该连接模块，其本质就是属于客户端模块，对于客户端而言，其实际是和信道直接关联的
        // 不过在该代码中弱化了客户端的概念
        void onBasicResponce(const muduo::net::TcpConnectionPtr& conn, const basicCommonResponcePtr& message, muduo::Timestamp) {
            // 首先要先获取信道
            Channel::ptr channel = _channels->get(message->cid());
            if (channel.get() == nullptr) {
                DLOG("没有找到对应的信道\n");
                return;
            }
            channel->putBasicResponce(message);
        }

        void onConsumeResponce(const muduo::net::TcpConnectionPtr& conn, const basicConsumeResponcePtr& message, muduo::Timestamp) {
            Channel::ptr channel = _channels->get(message->cid());
            if (channel.get() == nullptr) {
                DLOG("没有找到对应的信道\n");
                return;
            }
            // 然后将消息处理任务当道线程中
            _worker->pool.push([channel, message](){
                channel->consume(message);
            });            
        }

        void onUnknownMessage(const muduo::net::TcpConnectionPtr& conn, const MessagePtr& message, muduo::Timestamp) {
            LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
            conn->shutdown();
        }

        void onConnection(const muduo::net::TcpConnectionPtr&conn){
            if (conn->connected()) {
                _latch.countDown();//唤醒主线程中的阻塞
                _conn = conn;
            }else {
                //连接关闭时的操作
                _conn.reset();
            }
        }    

        void connect() {
            _client.connect();
            _latch.wait();      //阻塞等待，直到连接建立成功
        }
    public:
        using ptr = std::shared_ptr<Connection>;

        Connection(const std::string &sip, int sport, const AsyncWorker::ptr& worker)
            : _worker(worker),
              _latch(1), 
              _client(worker->loopthread.startLoop(), muduo::net::InetAddress(sip, sport), "Client"),
              _dispatcher(std::bind(&Connection::onUnknownMessage, this, std::placeholders::_1, 
                std::placeholders::_2, std::placeholders::_3)),
              _codec(std::make_shared<ProtobufCodec>(std::bind(&ProtobufDispatcher::onProtobufMessage, &_dispatcher, 
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))),
              _channels(std::make_shared<ChannelManager>())
        {


            _dispatcher.registerMessageCallback<basicCommonResponce>(std::bind(&Connection::onBasicResponce, this, 
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                    
            _dispatcher.registerMessageCallback<basicConsumeResponce>(std::bind(&Connection::onConsumeResponce, this, 
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

            _client.setMessageCallback(std::bind(&ProtobufCodec::onMessage, _codec.get(),
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

            _client.setConnectionCallback(std::bind(&Connection::onConnection, this, std::placeholders::_1)); 

            // 构造的时候就直接开始连接
            this->connect();     
        }

        Channel::ptr openChannel() {
            Channel::ptr channel = _channels->create(_conn, _codec);
            bool ret = channel->openChannel();
            if (ret == false) {
                DLOG("创建信道失败\n");
                return Channel::ptr();
            }
            return channel;
        }

        void closeChannel(const Channel::ptr& channel) {
            channel->closeChannel();
            _channels->remove(channel->cid());
        }

    private:
        muduo::CountDownLatch _latch;                           //实现同步的
        muduo::net::TcpConnectionPtr _conn;                     //客户端对应的连接
        muduo::net::TcpClient _client;                          //客户端
        ProtobufDispatcher _dispatcher;                         //请求分发器
        ProtobufCodecPtr _codec;                                //协议处理器   
        AsyncWorker::ptr _worker;                               // 任务处理线程 & IO事件监控线程
        ChannelManager::ptr _channels;                          // 信道管理 
    };

}

#endif