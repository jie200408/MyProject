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
#include "../MqCommon/admin.pb.h"
#include "../MqCommon/threadpool.hpp"
#include "../MqCommon/user.pb.h"
#include "host.hpp"
#include "consumer.hpp"
#include "connection.hpp"
#include "users.hpp"

namespace mq {

    #define DEFAULT_DBFILE "/meta.db"
    #define HOST_NAME "MyVirtualHost"

    class BrokerServer {
    private:
        using MessagePtr = std::shared_ptr<google::protobuf::Message>;
        using UserLoginInfoPtr = std::shared_ptr<userLogin>;
        using userLogoutInfoPtr = std::shared_ptr<userLogout>;
        using userInfoRequestPtr = std::shared_ptr<userInfoRequest>;
        using garbageRecivePtr = std::shared_ptr<garbageRecive>;
        using getExchangeTypeRequestPtr = std::shared_ptr<getExchangeTypeRequest>;

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

        void onGetExchangeType(const muduo::net::TcpConnectionPtr& conn, const getExchangeTypeRequestPtr& message, muduo::Timestamp) {
            Exchange::ptr ep = _virtual_host->selectExchange(message->exchange_name());
            getExchangeTypeResponce resp;
            resp.set_rid(message->rid());
            resp.set_cid(message->cid());
            if (ep.get() == nullptr) {
                resp.set_type(0);
            } else {
                if (ep->type == ExchangeType::DIRECT)
                    resp.set_type(1);
                else if (ep->type == ExchangeType::FANOUT)
                    resp.set_type(2);
                else 
                    resp.set_type(3);
            }
            // 发送出去
            _codec->send(conn, resp);
        }

        void onUserLogin(const muduo::net::TcpConnectionPtr& conn, const UserLoginInfoPtr& message) {
            auto ret = _users->login(message->username(), message->password());
            basicCommonResponce resp;
            resp.set_ok(ret.first);
            resp.set_cid(message->cid());
            resp.set_rid(message->rid());
            // 设置用户的类型
            UserType user_type = ret.second == 1 ? mq::RECIVER : mq::PUBLISHER;
            if (ret.second == 0)
                user_type = mq::ADMIN;
            resp.set_user_type(user_type);
            _codec->send(conn, resp);
        }

        void onUserRegister(const muduo::net::TcpConnectionPtr& conn, const UserLoginInfoPtr& message) {
            bool ret = _users->signIn(message->username(), message->password(), message->user_type());
            basicCommonResponce resp;
            resp.set_ok(ret);
            resp.set_cid(message->cid());
            resp.set_rid(message->rid());
            resp.set_user_type(message->user_type());
            _codec->send(conn, resp);
        }

        void onGarbageRecive(const muduo::net::TcpConnectionPtr& conn, const garbageRecivePtr& message, muduo::Timestamp) {
            _virtual_host->gc();
            basicCommonResponce resp;
            resp.set_cid(message->cid());
            resp.set_rid(message->rid());
            _codec->send(conn, resp);
        }

        void onUserLogout(const muduo::net::TcpConnectionPtr& conn, const userLogoutInfoPtr& message, muduo::Timestamp) {
            _users->logout(message->username());
            basicCommonResponce resp;
            resp.set_ok(true);
            resp.set_cid(message->cid());
            resp.set_rid(message->rid());
            resp.set_user_type(message->user_type());
            _codec->send(conn, resp);
        } 

        void onCheckUser(const muduo::net::TcpConnectionPtr& conn, const UserLoginInfoPtr& message, muduo::Timestamp) {
            // 先判断是否进行注册用户
            bool isRegister = message->isregister();
            if (!isRegister)
                onUserLogin(conn, message);
            else
                onUserRegister(conn,message);
            // 直接在服务器的第一层就拦截用户登陆
            // 只有当用户成功登陆才可以继续进行用能使用
        }      

        void onUserAllInfo(const muduo::net::TcpConnectionPtr& conn, const userInfoRequestPtr& message, muduo::Timestamp) {
            userInfoResponce resp;
            resp.set_cid(message->cid());
            resp.set_rid(message->rid());
            // 现在从底层将数据获取到
            std::string allusers = _users->getAllUsers();
            resp.set_user_infos(allusers);
            _codec->send(conn, resp);
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
                conn->shutdown();
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
              _threadpool(std::make_shared<threadpool>()),
              _users(std::make_shared<UserManager>(basedir + DEFAULT_DBFILE))
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
            
            _dispathcher.registerMessageCallback<userLogin>(std::bind(&BrokerServer::onCheckUser, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); 
            _dispathcher.registerMessageCallback<userLogout>(std::bind(&BrokerServer::onUserLogout, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); 

            _dispathcher.registerMessageCallback<userInfoRequest>(std::bind(&BrokerServer::onUserAllInfo, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); 

            _dispathcher.registerMessageCallback<garbageRecive>(std::bind(&BrokerServer::onGarbageRecive, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); 

            _dispathcher.registerMessageCallback<getExchangeTypeRequest>(std::bind(&BrokerServer::onGetExchangeType, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); 

            _server.setMessageCallback(std::bind(&ProtobufCodec::onMessage, _codec.get(), 
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _server.setConnectionCallback(std::bind(&BrokerServer::onConnection, this, std::placeholders::_1));
        }      

        void start() {
            // 服务器开始运行
            _server.start();
            // 开始IO监控
            _baseloop.loop();
        }

    private:
        muduo::net::TcpServer _server;              // 服务器对象
        muduo::net::EventLoop _baseloop;            // 主事件循环器，响应和监听IO事件
        ProtobufDispatcher _dispathcher;            // 请求分发器对象，需要向分发器中的注册处理函数
        ProtobufCodecPtr _codec;                    // protobuf 协议处理器，针对收到的请求数据进行protobuf协议处理 
        VirtualHost::ptr _virtual_host;             // 虚拟机
        ConsumerManager::ptr _consumer_manager;     // 消费者管理句柄
        ConnectionManager::ptr _connection_manager; // 连接管理句柄
        threadpool::ptr _threadpool;                // 线程池管理句柄

        UserManager::ptr _users;                    // 用户管理句柄
    };

}

#endif