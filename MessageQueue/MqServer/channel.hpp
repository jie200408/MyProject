#ifndef __M_CHANNEL_H__
#define __M_CHANNEL_H__

#include <muduo/net/TcpConnection.h>
#include "../MqThird/include/codec.h"
#include "../MqCommon/proto.pb.h"
#include "../MqCommon/msg.pb.h"
#include "../MqCommon/helper.hpp"
#include "../MqCommon/logger.hpp"
#include "../MqCommon/threadpool.hpp"
#include "consumer.hpp"
#include "host.hpp"
#include "route.hpp"

namespace mq {
    using ProtobufCodecPtr = std::shared_ptr<ProtobufCodec>;
    // 以下请求的智能指针全都是基于proto.pb.h中生成的信息管理类
    using openChannelRequestPtr = std::shared_ptr<openChannelRequest>;
    using closeChannelRequestPtr = std::shared_ptr<closeChannelRequest>;
    using declareExchangeRequestPtr = std::shared_ptr<declareExchangeRequest>;
    using deleteExchangeRequestPtr = std::shared_ptr<deleteExchangeRequest>;
    using declareQueueRequestPtr = std::shared_ptr<declareQueueRequest>;
    using deleteQueueRequestPtr = std::shared_ptr<deleteQueueRequest>;
    using queueBindRequestPtr = std::shared_ptr<queueBindRequest>;
    using queueUnBindRequestPtr = std::shared_ptr<queueUnBindRequest>;
    using basicPublishRequestPtr = std::shared_ptr<basicPublishRequest>;
    using basicAckRequestPtr = std::shared_ptr<basicAckRequest>;
    using basicCancelRequestPtr = std::shared_ptr<basicCancelRequest>;
    using basicConsumeResponcePtr = std::shared_ptr<basicConsumeResponce>;
    using basicConsumeRequestPtr = std::shared_ptr<basicConsumeRequest>;

    class Channel {
    private:
        // 基础响应，将响应发回给客户端
        void basicResponce(bool ok, const std::string& rid, const std::string& cid) {
            basicCommonResponce resp;
            // 设置响应的各个参数
            resp.set_cid(cid);
            resp.set_rid(rid);
            resp.set_ok(ok);
            _codec->send(_conn, resp);
        }
        
        // 使用这个作为回调函数进行消息消费
        void consume(const std::string& qname) {
            // 1. 取出一个消息
            MessagePtr mp = _host->basicConsume(qname);
            if (mp.get() == nullptr) {
                DLOG("消费消息失败，%s 队列没有可以消费的消息\n", qname.c_str());
                return;
            }
            // 2. 取出一个消费者
            Consumer::ptr cp = _cmp->choose(qname);
            if (cp.get() == nullptr) {
                DLOG("消费消息失败，%s 队列没有消费者\n", qname.c_str());
                return;
            }
            // 进行消息消费
            cp->callback(cp->tag, mp->mutable_payload()->mutable_properties(), mp->payload().body());
            // 若当前为自动删除，则直接将消息给删除了，否则需要之后手动删除
            if (cp->auto_ack)
                _host->basicAck(qname, mp->payload().properties().id());
        }

        // 消息处理回调函数
        void callback(const std::string& tag, const BasicProperties* bp, const std::string& body) {
            basicConsumeResponce resp;
            resp.set_body(body);
            resp.set_cid(_cid);
            resp.set_consumer_tag(tag);
            if (bp) {
                resp.mutable_properties()->set_id(bp->id());
                resp.mutable_properties()->set_routing_key(bp->routing_key());
                resp.mutable_properties()->set_delivery_mode(bp->delivery_mode());
            }
            _codec->send(_conn, resp);
        }

    public:
        using ptr = std::shared_ptr<Channel>;

        Channel(const std::string& id, 
            const VirtualHost::ptr& host, 
            const ConsumerManager::ptr& cmp, 
            const ProtobufCodecPtr& codec, 
            const muduo::net::TcpConnectionPtr conn, 
            const threadpool::ptr& pool)
            : _cid(id),
              _conn(conn),
              _codec(codec),
              _cmp(cmp),
              _host(host),
              _pool(pool)
        {}
        
        // 交换机声明
        void declareExchange(const declareExchangeRequestPtr& req) {
            bool ret = _host->declareExchange(req->exchange_name(), req->exchange_type(), 
                req->durable(), req->auto_delete(), req->args());
            basicResponce(ret, req->rid(), req->cid());
        }

        // 删除交换机
        void deleteExchange(const deleteExchangeRequestPtr& req) {
            _host->deleteExchange(req->exchange_name());
            basicResponce(true, req->rid(), req->cid());
        }

        // 队列声明
        void declareQueue(const declareQueueRequestPtr& req) {
            bool ret = _host->declareQueue(req->queue_name(), 
            req->durable(), req->exclusive(), req->auto_delete(), req->args());
            if (ret == false) 
                return basicResponce(ret, req->rid(), req->cid());
            _cmp->initQueueConsumer(req->queue_name());
            basicResponce(ret, req->rid(), req->cid());
        }

        // 删除队列
        void deleteQueue(const deleteQueueRequestPtr& req) {
            _host->deleteQueue(req->queue_name());
            _cmp->destroyQueueConsumer(req->queue_name());
            basicResponce(true, req->rid(), req->cid());
        }

        // 绑定
        void bind(const queueBindRequestPtr& req) {
            bool ret = _host->bind(req->exchange_name(), req->queue_name(), req->binding_key());
            basicResponce(ret, req->rid(), req->cid());
        }

        // 解绑
        void unBind(const queueUnBindRequestPtr& req) {
            _host->unBind(req->exchange_name(), req->queue_name());
            basicResponce(true, req->rid(), req->cid());
        }

        // 发布消息
        void basicPublish(const basicPublishRequestPtr& req) {
            // 取出一个交换机
            Exchange::ptr ep = _host->selectExchange(req->exchange_name());
            if (ep.get() == nullptr) 
                return basicResponce(false, req->rid(), req->cid());
            // 根据获取的交换机找到对应的绑定信息
            BasicProperties* bp = nullptr;
            std::string routing_key;
            if (req->has_properties()) {
                bp = req->mutable_properties();
                routing_key = req->properties().routing_key();
            }

            MsgQueueBindingMap mqbm = _host->exchangeBindings(req->exchange_name());
            for (auto& binding : mqbm) {
                if (Router::route(ep->type, routing_key, binding.second->binding_key)) {
                    // 将消息加入到队列中
                    _host->basicPublish(binding.first, bp, req->body());

                    auto task = std::bind(&Channel::consume, this, binding.first);
                    _pool->push(task);
                }
            }
            basicResponce(true, req->rid(), req->cid()); 
        }

        // 确认消息
        void basicAck(const basicAckRequestPtr& req) {
            _host->basicAck(req->queue_name(), req->message_id());
            basicResponce(true, req->rid(), req->cid()); 
        }

        // 订阅消息
        void basicConsume(const basicConsumeRequestPtr& req) {
            // 判断当前队列是否存在
            bool ret = _host->existsQueue(req->queue_name());
            if (ret == false)
                return basicResponce(false, req->rid(), req->cid()); 

            auto cb = std::bind(&Channel::callback, this, std::placeholders::_1, 
                std::placeholders::_2, std::placeholders::_3);    
            _consumer = _cmp->create(req->consumer_tag(), req->queue_name(), req->auto_ack(), cb);
            if (_consumer.get() == nullptr)
                return basicResponce(false, req->rid(), req->cid()); 
            basicResponce(true, req->rid(), req->cid()); 
        }

        // 取消订阅
        void basicCancel(const basicCancelRequestPtr& req) {
            // 取消订阅就是将消费者从消费者管理句柄中删除
            _cmp->remove(req->queue_name(), req->consumer_tag());
            basicResponce(true, req->rid(), req->cid()); 
        }

        ~Channel() {
            if (_consumer.get() != nullptr)
                _cmp->remove(_consumer->qname, _consumer->tag);
        }
    private:
        std::string _cid;                          // 信道唯一标识
        Consumer::ptr _consumer;                   // 信道关联的消费者
        muduo::net::TcpConnectionPtr _conn;        // 信道关联的连接
        ProtobufCodecPtr _codec;                   // 协议处理器，protobuf协议处理句柄  
        ConsumerManager::ptr _cmp;                 // 消费者管理句柄
        VirtualHost::ptr _host;                    // 虚拟机
        threadpool::ptr _pool;                     // 线程池
    };

    class ChannelManager {
    public:
    using ptr = std::shared_ptr<ChannelManager>;

    ChannelManager() {
        ILOG("new Channel %p\n", this);
    }

    bool openChannel(const std::string& cid, 
            const VirtualHost::ptr& host, 
            const ConsumerManager::ptr& cmp, 
            const ProtobufCodecPtr& codec, 
            const muduo::net::TcpConnectionPtr conn, 
            const threadpool::ptr& pool) {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _channels.find(cid);
        if (it != _channels.end())
            return false;
        Channel::ptr channel = std::make_shared<Channel>(cid, host, cmp, codec, conn, pool);
        _channels[cid] = channel;
        return true;
    }

    void closeChannel(const std::string& cid) {
        std::unique_lock<std::mutex> lock(_mutex);
        _channels.erase(cid);
    }

    Channel::ptr getChannel(const std::string& cid) {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _channels.find(cid);
        if (it == _channels.end())
            return Channel::ptr();
        return it->second;        
    }

    ~ChannelManager() {
        ILOG("del Channel %p\n", this);
    }

    private:
        std::mutex _mutex;
        std::unordered_map<std::string, Channel::ptr> _channels;
    };
}

#endif