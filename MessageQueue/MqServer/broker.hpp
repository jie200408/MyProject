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

    #define DEFAULT_DBFILE "/meta.db"
    #define HOST_NAME "MyVirtualHost"

    class BrokerServer {
    private:
        using MessagePtr = std::shared_ptr<google::protobuf::Message>;;

        void onConnection(const muduo::net::TcpConnectionPtr& conn) {
            
            if (conn->connected()) 
                _connection_manager->newConnection(_virtual_host, _consumer_manager, _codec, conn, _threadpool);
            else 
                _connection_manager->delConnection(conn);           
        }
        // 默认的处理函数
        void unUnKnownMessage(const muduo::net::TcpConnectionPtr& conn, const MessagePtr& message, muduo::Timestamp) {
            LOG_INFO << "unUnKnownMessage" << message->GetTypeName();
            conn->shutdown(); // 关闭连接
        }

        // 打开信道
        void onOpenChannel(const muduo::net::TcpConnectionPtr& conn, const openChannelRequestPtr& message, muduo::Timestamp) {
            Connection::ptr mconn = _connection_manager->getConnection(conn);
            if (mconn.get() == nullptr) {
                DLOG("打开信道时，没有找到对应的连接\n");
                return;
            }
            mconn->openChannel(message);
        }

        void onCloseChannel(const muduo::net::TcpConnectionPtr& conn, const closeChannelRequestPtr& message, muduo::Timestamp) {
            Connection::ptr mconn = _connection_manager->getConnection(conn);
            if (mconn.get() == nullptr) {
                DLOG("关闭信道时，没有找到对应的连接\n");
                return;
            }
            mconn->closeChannel(message);
        }

        void onDeclareExchange(const muduo::net::TcpConnectionPtr& conn, const declareExchangeRequestPtr& message, muduo::Timestamp) {
            Connection::ptr mconn = _connection_manager->getConnection(conn);
            if (mconn.get() == nullptr) {
                DLOG("声明交换机时，没有找到对应的连接\n");
                return;
            }
            Channel::ptr channel = mconn->getChannel(message->cid());
            if (channel.get() == nullptr) {
                DLOG("声明交换机时，没有找到对应的信道\n");
                return;
            }          
            channel->declareExchange(message); 
        }

        void onDeleteExchange(const muduo::net::TcpConnectionPtr& conn, const deleteExchangeRequestPtr& message, muduo::Timestamp) {
            Connection::ptr mconn = _connection_manager->getConnection(conn);
            if (mconn.get() == nullptr) {
                DLOG("关闭交换机时，没有找到对应的连接\n");
                return;
            }
            Channel::ptr channel = mconn->getChannel(message->cid());
            if (channel.get() == nullptr) {
                DLOG("关闭交换机时，没有找到对应的信道\n");
                return;
            }          
            channel->deleteExchange(message);         
        }
        
        void onDeclareQueue(const muduo::net::TcpConnectionPtr& conn, const declareQueueRequestPtr& message, muduo::Timestamp) {
            Connection::ptr mconn = _connection_manager->getConnection(conn);
            if (mconn.get() == nullptr) {
                DLOG("声明队列时，没有找到对应的连接\n");
                return;
            }
            Channel::ptr channel = mconn->getChannel(message->cid());
            if (channel.get() == nullptr) {
                DLOG("声明队列时，没有找到对应的信道\n");
                return;
            }          
            channel->declareQueue(message);         
        }

        void onDeleteQueue(const muduo::net::TcpConnectionPtr& conn, const deleteQueueRequestPtr& message, muduo::Timestamp) {
            Connection::ptr mconn = _connection_manager->getConnection(conn);
            if (mconn.get() == nullptr) {
                DLOG("关闭队列时，没有找到对应的连接\n");
                return;
            }
            Channel::ptr channel = mconn->getChannel(message->cid());
            if (channel.get() == nullptr) {
                DLOG("关闭队列时，没有找到对应的信道\n");
                return;
            }          
            channel->deleteQueue(message);            
        }

        void onQueueBind(const muduo::net::TcpConnectionPtr& conn, const queueBindRequestPtr& message, muduo::Timestamp) {
            Connection::ptr mconn = _connection_manager->getConnection(conn);
            if (mconn.get() == nullptr) {
                DLOG("绑定队列时，没有找到对应的连接\n");
                return;
            }
            Channel::ptr channel = mconn->getChannel(message->cid());
            if (channel.get() == nullptr) {
                DLOG("绑定队列时，没有找到对应的信道\n");
                return;
            }          
            channel->bind(message);           
        }

        void onQueueUnBind(const muduo::net::TcpConnectionPtr& conn, const queueUnBindRequestPtr& message, muduo::Timestamp) {
            Connection::ptr mconn = _connection_manager->getConnection(conn);
            if (mconn.get() == nullptr) {
                DLOG("解绑队列时，没有找到对应的连接\n");
                return;
            }
            Channel::ptr channel = mconn->getChannel(message->cid());
            if (channel.get() == nullptr) {
                DLOG("解绑队列时，没有找到对应的信道\n");
                return;
            }          
            channel->unBind(message);          
        }

        void onBasicPublish(const muduo::net::TcpConnectionPtr& conn, const basicPublishRequestPtr& message, muduo::Timestamp) {
            Connection::ptr mconn = _connection_manager->getConnection(conn);
            if (mconn.get() == nullptr) {
                DLOG("发布消息时，没有找到对应的连接\n");
                return;
            }
            Channel::ptr channel = mconn->getChannel(message->cid());
            if (channel.get() == nullptr) {
                DLOG("发布消息时，没有找到对应的信道\n");
                return;
            }          
            channel->basicPublish(message);             
        }

        void onBasicAck(const muduo::net::TcpConnectionPtr& conn, const basicAckRequestPtr& message, muduo::Timestamp) {
            Connection::ptr mconn = _connection_manager->getConnection(conn);
            if (mconn.get() == nullptr) {
                DLOG("确认消息时，没有找到对应的连接\n");
                return;
            }
            Channel::ptr channel = mconn->getChannel(message->cid());
            if (channel.get() == nullptr) {
                DLOG("确认消息时，没有找到对应的信道\n");
                return;
            }          
            channel->basicAck(message);          
        }
    
        void onBasicConsume(const muduo::net::TcpConnectionPtr& conn, const basicConsumeRequestPtr& message, muduo::Timestamp) {
            Connection::ptr mconn = _connection_manager->getConnection(conn);
            if (mconn.get() == nullptr) {
                DLOG("订阅消息时，没有找到对应的连接\n");
                return;
            }
            Channel::ptr channel = mconn->getChannel(message->cid());
            if (channel.get() == nullptr) {
                DLOG("订阅消息时，没有找到对应的信道\n");
                return;
            }          
            channel->basicConsume(message); 
        }

        void onBasicCancel(const muduo::net::TcpConnectionPtr& conn, const basicCancelRequestPtr& message, muduo::Timestamp) {
            Connection::ptr mconn = _connection_manager->getConnection(conn);
            if (mconn.get() == nullptr) {
                DLOG("取消订阅时，没有找到对应的连接\n");
                return;
            }
            Channel::ptr channel = mconn->getChannel(message->cid());
            if (channel.get() == nullptr) {
                DLOG("取消订阅时，没有找到对应的信道\n");
                return;
            }          
            channel->basicCancel(message);         
        }
    public:
        BrokerServer(uint16_t port, const std::string& basedir)
            : _server(&_baseloop, muduo::net::InetAddress("0.0.0.0", port), "Server", muduo::net::TcpServer::kReusePort),
              _dispathcher(std::bind(&BrokerServer::unUnKnownMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
              _codec(std::make_shared<ProtobufCodec>(std::bind(&ProtobufDispatcher::onProtobufMessage, 
                &_dispathcher, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))),
              _virtual_host(std::make_shared<VirtualHost>(HOST_NAME, basedir, basedir + DEFAULT_DBFILE)),
              _consumer_manager(std::make_shared<ConsumerManager>()),
              _connection_manager(std::make_shared<ConnectionManager>()),
              _threadpool(std::make_shared<threadpool>())
        {
            // 恢复历史队列消息
            MsgQueueMapper::MsgQueueMap qmap = _virtual_host->allQueue();
            for (auto& q : qmap)
                _consumer_manager->initQueueConsumer(q.first);
            // 现在注册业务处理请求函数
            _dispathcher.registerMessageCallback<openChannelRequest>(std::bind(&BrokerServer::onOpenChannel, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _dispathcher.registerMessageCallback<closeChannelRequest>(std::bind(&BrokerServer::onCloseChannel, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));         

            _dispathcher.registerMessageCallback<declareExchangeRequest>(std::bind(&BrokerServer::onDeclareExchange, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));   
            _dispathcher.registerMessageCallback<deleteExchangeRequest>(std::bind(&BrokerServer::onDeleteExchange, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); 

            _dispathcher.registerMessageCallback<declareQueueRequest>(std::bind(&BrokerServer::onDeclareQueue, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));   
            _dispathcher.registerMessageCallback<deleteQueueRequest>(std::bind(&BrokerServer::onDeleteQueue, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); 
            
            _dispathcher.registerMessageCallback<queueBindRequest>(std::bind(&BrokerServer::onQueueBind, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));   
            _dispathcher.registerMessageCallback<queueUnBindRequest>(std::bind(&BrokerServer::onQueueUnBind, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); 

            _dispathcher.registerMessageCallback<basicPublishRequest>(std::bind(&BrokerServer::onBasicPublish, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));                                   
            _dispathcher.registerMessageCallback<basicAckRequest>(std::bind(&BrokerServer::onBasicAck, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); 
            _dispathcher.registerMessageCallback<basicConsumeRequest>(std::bind(&BrokerServer::onBasicConsume, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));                 
            _dispathcher.registerMessageCallback<basicCancelRequest>(std::bind(&BrokerServer::onBasicCancel, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); 
            
            
            
            _server.setMessageCallback(std::bind(&ProtobufCodec::onMessage, _codec.get(), 
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
        ProtobufCodecPtr _codec;                       // protobuf 协议处理器，针对收到的请求数据进行protobuf协议处理 
        VirtualHost::ptr _virtual_host;             // 虚拟机
        ConsumerManager::ptr _consumer_manager;     // 消费者管理句柄
        ConnectionManager::ptr _connection_manager; // 连接管理句柄
        threadpool::ptr _threadpool;                // 线程池管理句柄
    };

}

#endif