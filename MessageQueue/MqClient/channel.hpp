#ifndef __M_CHANNEL_H__
#define __M_CHANNEL_H__

#include "../MqThird/include/codec.h"
#include "../MqCommon/proto.pb.h"
#include "../MqCommon/msg.pb.h"
#include "../MqCommon/user.pb.h"
#include "../MqCommon/admin.pb.h"
#include "../MqCommon/helper.hpp"
#include "../MqCommon/logger.hpp"
#include "../MqCommon/threadpool.hpp"
#include "consumer.hpp"
#include <muduo/net/TcpConnection.h>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <unordered_map>


namespace mq {
    using ProtobufCodecPtr = std::shared_ptr<ProtobufCodec>;
    using basicConsumeResponcePtr = std::shared_ptr<basicConsumeResponce>;
    using basicCommonResponcePtr = std::shared_ptr<basicCommonResponce>;
    using MessagePtr = std::shared_ptr<google::protobuf::Message>;
    using UserInfoResponcePtr = std::shared_ptr<userInfoResponce>;
    using getExchangeTypeResponcePtr = std::shared_ptr<getExchangeTypeResponce>;

    class Channel {
    private:
        // 基础响应
        basicCommonResponcePtr waitResponce(const std::string& rid) {
            std::unique_lock<std::mutex> lock(_mutex);
            _cond.wait(lock, [&rid, this]() {
                return _basic_resp.find(rid) != _basic_resp.end();
            });
            basicCommonResponcePtr resp = _basic_resp[rid];
            _basic_resp.erase(rid);
            return resp;
        }

        // 登陆操作
        basicCommonResponcePtr load(const std::string& username, const std::string& password, bool isRegister, UserType user_type) {
            userLogin user;
            std::string rid = UUIDHelper::uuid();
            user.set_password(password);
            user.set_username(username);
            user.set_cid(_channel_id);
            user.set_rid(rid);
            user.set_isregister(isRegister);
            if (isRegister == true) 
                user.set_user_type(user_type);
            _codec->send(_conn, user);
            basicCommonResponcePtr resp = waitResponce(rid);
            return resp;
        }

    public:
        using ptr = std::shared_ptr<Channel>;

        Channel(const muduo::net::TcpConnectionPtr& conn, const ProtobufCodecPtr& codec)
            : _channel_id(UUIDHelper::uuid()),
              _conn(conn),
              _codec(codec)
        {}

        bool declareExchange(const std::string& ename, ExchangeType etype, bool edurable, 
            bool eauto_delete, google::protobuf::Map<std::string, std::string>& eargs) {
            // 1. 构建对象
            std::string rid = UUIDHelper::uuid();
            declareExchangeRequest req;
            req.set_cid(_channel_id);
            req.set_rid(rid);
            req.set_durable(edurable);
            req.set_exchange_name(ename);
            req.set_exchange_type(etype);
            req.set_auto_delete(eauto_delete);
            req.mutable_args()->swap(eargs);
            // 2. 将构建的对象发送出去
                // protobuf底层设计了自己的发送和接收缓冲区，发送和接收是异步工作的
                // 所以只有对方确认收到（等待响应）之后我们才可以返回 
            _codec->send(_conn, req);
            // 3. 等待响应
            basicCommonResponcePtr resp = waitResponce(rid);
            // 4. 返回
            return resp->ok();
        }

        void deleteExchange(const std::string& ename) {
            std::string rid = UUIDHelper::uuid();
            deleteExchangeRequest req;
            req.set_cid(_channel_id);
            req.set_rid(rid);
            req.set_exchange_name(ename);
            _codec->send(_conn, req);
            waitResponce(rid);
        }

        bool declareQueue(const std::string& qname, bool qdurable, 
            bool qexclusive, bool qauto_delete, 
            google::protobuf::Map<std::string, std::string>& qargs) {
            
            std::string rid = UUIDHelper::uuid();
            declareQueueRequest req;
            req.set_cid(_channel_id);
            req.set_rid(rid);
            req.set_durable(qdurable);
            req.set_queue_name(qname);
            req.set_auto_delete(qauto_delete);
            req.mutable_args()->swap(qargs);    
            req.set_exclusive(qexclusive);      

            _codec->send(_conn, req);
            // 3. 等待响应
            basicCommonResponcePtr resp = waitResponce(rid);
            // 4. 返回
            return resp->ok();              
        }

        void deleteQueue(const std::string& qname) {
            std::string rid = UUIDHelper::uuid();
            deleteQueueRequest req;
            req.set_cid(_channel_id);
            req.set_rid(rid);    
            req.set_queue_name(qname);
            _codec->send(_conn, req);

            waitResponce(rid);
        }

        bool queueBind(const std::string& ename, const std::string& qname, const std::string& key) {
            std::string rid = UUIDHelper::uuid();
            queueBindRequest req;
            req.set_cid(_channel_id);
            req.set_rid(rid);    
            req.set_queue_name(qname);    
            req.set_exchange_name(ename);
            req.set_binding_key(key);   

            _codec->send(_conn, req);
            // 3. 等待响应
            basicCommonResponcePtr resp = waitResponce(rid);
            // 4. 返回
            return resp->ok();         
        }

        void queueUnBind(const std::string& ename, const std::string& qname) {
            std::string rid = UUIDHelper::uuid();
            queueUnBindRequest req;
            req.set_cid(_channel_id);
            req.set_rid(rid);    
            req.set_queue_name(qname);    
            req.set_exchange_name(ename);
            _codec->send(_conn, req);

            waitResponce(rid);          
        }

        void basicPublish(const std::string& ename, const BasicProperties* bp, const std::string& body) {
            // 发送消息给交换机，让交换机来自动匹配消息发给哪个队列
            std::string rid = UUIDHelper::uuid();
            basicPublishRequest req;
            req.set_cid(_channel_id);
            req.set_rid(rid);    
            req.set_exchange_name(ename);
            req.set_body(body);
            if (bp != nullptr) {
                req.mutable_properties()->set_id(bp->id());
                req.mutable_properties()->set_delivery_mode(bp->delivery_mode());
                req.mutable_properties()->set_routing_key(bp->routing_key());
            }
            _codec->send(_conn, req);

            waitResponce(rid);          
        }

        std::pair<bool, UserType> loadPublish(const std::string& username, const std::string& password, UserType user_type) {
            basicCommonResponcePtr resp = this->load(username, password, false, user_type);
            // 返回类型和是否登陆成功
            return std::make_pair(resp->ok(), resp->user_type());
        }

        std::pair<bool, UserType> registerPublish(const std::string& username, const std::string& password, UserType user_type) {
            basicCommonResponcePtr resp = this->load(username, password, true, user_type);
            return std::make_pair(resp->ok(), resp->user_type());
        }

        void logoutPublish(const std::string& username, UserType user_type) {
            userLogout req;
            std::string rid = UUIDHelper::uuid();
            req.set_cid(_channel_id);
            req.set_rid(rid);
            req.set_user_type(user_type);
            req.set_username(username);
            _codec->send(_conn, req);
            waitResponce(rid);    
        }

        void getAllUsersPublish() {
            userInfoRequest req;
            std::string rid = UUIDHelper::uuid();
            req.set_cid(_channel_id);
            req.set_rid(rid);
            _codec->send(_conn, req);
        }

        void getExchangeTypePublish(const std::string& ename) {
            getExchangeTypeRequest req;
            std::string rid = UUIDHelper::uuid();
            req.set_cid(_channel_id);
            req.set_rid(rid);
            req.set_exchange_name(ename);
            _codec->send(_conn, req);
        }

        void garbageRecivePublish() {
            garbageRecive req;
            std::string rid = UUIDHelper::uuid();
            req.set_cid(_channel_id);
            req.set_rid(rid);
            _codec->send(_conn, req);
            waitResponce(rid);  
        }

        void basicAck(const std::string& msg_id) {
            if (_consumer.get() == nullptr) {
                DLOG("确认消息时，找不到对应的消费者信息\n");
                return;
            }
            std::string rid = UUIDHelper::uuid();
            basicAckRequest req;
            req.set_cid(_channel_id);
            req.set_rid(rid);           
            req.set_queue_name(_consumer->qname);
            req.set_message_id(msg_id);     
            _codec->send(_conn, req);

            waitResponce(rid);          
        }

        // 订阅消息
        bool basicConsume(const std::string& consumer_tag, 
            const std::string& qname, bool auto_ack, const ConsumerCallback& cb) {
            if (_consumer.get() != nullptr) {
                DLOG("消费者已经存在，不用订阅\n");
                return false;
            }
            std::string rid = UUIDHelper::uuid();
            basicConsumeRequest req;
            req.set_rid(rid);
            req.set_cid(_channel_id);
            req.set_queue_name(qname);
            req.set_consumer_tag(consumer_tag);
            req.set_auto_ack(auto_ack);
            _codec->send(_conn, req);

            basicCommonResponcePtr resp = waitResponce(rid);
            if (resp->ok() == false) {
                DLOG("添加订阅失败\n");
                return false;
            }
            // 生成当前信道对应的消费者
            _consumer = std::make_shared<Consumer>(consumer_tag, qname, auto_ack, cb);
            return true;
        }

        void basicCancel() {
            if (_consumer.get() == nullptr)
                return;
            std::string rid = UUIDHelper::uuid();
            basicCancelRequest req;
            req.set_cid(_channel_id);
            req.set_rid(rid);
            req.set_consumer_tag(_consumer->tag);
            req.set_queue_name(_consumer->qname);
            _codec->send(_conn, req);

            waitResponce(rid);
            // 取消订阅，也就将当前的消费者重置          
            _consumer.reset();
        }

        std::string cid() {
            return _channel_id;
        }

        ~Channel() {
            basicCancel();
        }

        void setArgs(const std::string& args) {
            {
                std::lock_guard<std::mutex> lock(_mutex);
                _other_args = args;
            }
            _cond.notify_all(); // 通知等待线程条件已满足
        }

    public:
        // 收到消息之后，向对应响应消息队列中加入响应消息
        void putBasicResponce(const basicCommonResponcePtr& resp) {
            std::unique_lock<std::mutex> lock(_mutex);
            _basic_resp[resp->rid()] = resp;
            // 从外部接收到消息之后，唤醒之前等待的线程
            _cond.notify_all();
        }

        // 收到响应之后，需要找到对应的消费者去处理消息
        void consume(const basicConsumeResponcePtr& resp) {
            if (_consumer.get() == nullptr) {
                DLOG("处理消息时，订阅者为找到\n");
                return;
            }
            if (_consumer->tag != resp->consumer_tag()) {
                DLOG("处理消息时，订阅者和请求消息不对应\n");
                return;
            }
            _consumer->callback(resp->consumer_tag(), resp->mutable_properties(), resp->body());
        }

        void getAllUsers(const UserInfoResponcePtr& resp) {
            this->setArgs(resp->user_infos());
        }

        void getExchangeType(const getExchangeTypeResponcePtr& resp) {
            this->setArgs(std::to_string(resp->type()));
        }

        bool openChannel() {
            std::string rid = UUIDHelper::uuid();
            openChannelRequest req;
            req.set_cid(_channel_id);
            req.set_rid(rid);
            _codec->send(_conn, req);
            basicCommonResponcePtr resp = waitResponce(rid);
            return resp->ok();
        }

        void closeChannel() {
            std::string rid = UUIDHelper::uuid();
            closeChannelRequest req;
            req.set_cid(_channel_id);
            req.set_rid(rid);
            _codec->send(_conn, req);  
            waitResponce(rid);         
        }

        std::string getArgs() {
            std::unique_lock<std::mutex> lock(_mutex);
            _cond.wait(lock, [this]() { return !_other_args.empty(); }); // 等待条件满足
            std::string ret(_other_args);
            _other_args.clear();    
            return ret;
        }
    private:
        std::string _channel_id;                                                // 信道id
        muduo::net::TcpConnectionPtr _conn;                                     // 信道关联的网络通信对象
        ProtobufCodecPtr _codec;                                                // 协议处理对象
        Consumer::ptr _consumer;                                                // 信道关联的消费者
        std::mutex _mutex;                                                      // 锁：和条件变量共同维护响应和处理的先后顺序
        std::condition_variable _cond;                                          // 条件变量 
        std::unordered_map<std::string, basicCommonResponcePtr> _basic_resp;    // <req_id(rid), resp> 请求对应的响应信息队列
    
        std::string _other_args;
    };

    class ChannelManager {
    public:
        using ptr = std::shared_ptr<ChannelManager>;

        ChannelManager() {}

        // 创建信道
        Channel::ptr create(const muduo::net::TcpConnectionPtr& conn, const ProtobufCodecPtr& codec) {
            std::unique_lock<std::mutex> lock(_mutex);
            Channel::ptr channel = std::make_shared<Channel>(conn, codec);
            std::string cid = channel->cid();
            _channels[cid] = channel;
            return channel;
        }

        // 移除信道
        void remove(const std::string& cid) {
            std::unique_lock<std::mutex> lock(_mutex);
            _channels.erase(cid);
        }

        // 获取指定的队列
        Channel::ptr get(const std::string& cid) {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _channels.find(cid);
            if (it == _channels.end())
                return Channel::ptr();
            return it->second;
        }

    private:
        std::mutex _mutex;
        std::unordered_map<std::string, Channel::ptr> _channels;
    };
}

#endif