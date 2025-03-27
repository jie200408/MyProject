#ifndef __M_CONSUMER_H__
#define __M_CONSUMER_H__

#include "../MqCommon/msg.pb.h"
#include "../MqCommon/helper.hpp"
#include "../MqCommon/logger.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace mq {
    using ConsumerCallback = std::function<void(const std::string&, const BasicProperties* bp, const std::string&)>;

    struct Consumer {
        using ptr = std::shared_ptr<Consumer>;

        std::string tag;            // 消费者标识
        std::string qname;          // 消费者绑定队列名称
        bool auto_ack;              // 是否自动确认
        ConsumerCallback callback;  // 回调函数

        Consumer() {
            ILOG("new Consumer %p\n", this);
        }

        Consumer(const std::string& ctag, const std::string& queue_name, bool ack, const ConsumerCallback& cb)
            : tag(ctag),
              qname(queue_name),
              auto_ack(ack),
              callback(cb)
        {
            ILOG("new Consumer %p\n", this);
        }

        ~Consumer() {
            ILOG("del Consumer %p\n", this);
        }
    };

    class QueueConsumer {
    public: 
        using ptr = std::shared_ptr<QueueConsumer>;

        QueueConsumer(const std::string qname)
            : _qname(qname),
              _rr_seq(0)
        {}

        // 队列新增一个消费者对象
        Consumer::ptr create(const std::string& ctag, const std::string& queue_name, bool ack, const ConsumerCallback& cb) {
            std::unique_lock<std::mutex> lock(_mutex);
            // 先遍历查找当前是否已经存在
            for (auto& consumer : _consumers) {
                // 若已经存在则直接返回一个空的指针
                if (consumer->tag == ctag)
                    return Consumer::ptr();
            }
            // 构造对象
            Consumer::ptr consumer = std::make_shared<Consumer>(ctag, queue_name, ack, cb);
            _consumers.push_back(consumer);
            return consumer;
        }

        // 从队列中移除一个消费者对象
        void remove(const std::string& ctag) {
            std::unique_lock<std::mutex> lock(_mutex);
            for (auto it = _consumers.begin(); it != _consumers.end(); ++it) {
                // 找到对应的元素然后删除
                if ((*it)->tag == ctag) {
                    _consumers.erase(it);
                    return;
                }
            }
        }

        Consumer::ptr choose() {
            std::unique_lock<std::mutex> lock(_mutex);
            // 若没有消费者则直接返回
            if (_consumers.empty())
                return Consumer::ptr();
            // 轮转选取出一个消费者消费消息
            int index = _rr_seq % _consumers.size();
            _rr_seq++;
            return _consumers[index];
        }

        bool exists(const std::string& ctag) {
            std::unique_lock<std::mutex> lock(_mutex);
            // 循环遍历每个用户
            for (auto& consumer : _consumers) {
                if (consumer->tag == ctag)
                    return true;
            }     
            return false;       
        }

        void clear() {
            std::unique_lock<std::mutex> lock(_mutex);
            _rr_seq = 0;
            _consumers.clear();
        }
        
        bool empty() {
            std::unique_lock<std::mutex> lock(_mutex);
            return _consumers.empty();
        }


    private:
        std::mutex _mutex;                  // 锁
        std::string _qname;                 // 队列名称
        uint64_t _rr_seq;                   // 轮转序号
        std::vector<Consumer::ptr> _consumers;   // 消费者管理
    };


    class ConsumerManager {
    public:
        using ptr = std::shared_ptr<ConsumerManager>;

        ConsumerManager() {
        }

        void initQueueConsumer(const std::string& qname) {
            std::unique_lock<std::mutex> lock(_mutex);
            // 查找当前管理的队列中是否已经存在
            auto it = _qconsumers.find(qname);
            if (it != _qconsumers.end())    
                return;
            QueueConsumer::ptr qcp = std::make_shared<QueueConsumer>(qname);
            _qconsumers[qname] = qcp;
        }

        void destroyQueueConsumer(const std::string& qname) {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _qconsumers.find(qname);
            // 找不到直接退出
            if (it == _qconsumers.end())    
                return;  
            _qconsumers.erase(qname);          
        }

        Consumer::ptr create(const std::string& ctag, const std::string& qname, bool ack, const ConsumerCallback& cb) {
            QueueConsumer::ptr qcp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _qconsumers.find(qname);
                if (it == _qconsumers.end()) {
                    DLOG("没有找到队列: %s 的管理句柄\n", qname.c_str());
                    return Consumer::ptr();
                }
                qcp = it->second;
            }
            return qcp->create(ctag, qname, ack, cb);
        }

        void remove(const std::string& qname, const std::string& ctag) {
            QueueConsumer::ptr qcp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _qconsumers.find(qname);
                if (it == _qconsumers.end()) {
                    DLOG("没有找到队列: %s 的管理句柄\n", qname.c_str());
                    return;
                }
                qcp = it->second;
            }
            qcp->remove(ctag);           
        }

        Consumer::ptr choose(const std::string& qname) {
            QueueConsumer::ptr qcp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _qconsumers.find(qname);
                if (it == _qconsumers.end()) {
                    DLOG("没有找到队列: %s 的管理句柄\n", qname.c_str());
                    return Consumer::ptr();
                }
                qcp = it->second;
            }
            return qcp->choose();              
        }

        bool empty(const std::string& qname) {
            QueueConsumer::ptr qcp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _qconsumers.find(qname);
                if (it == _qconsumers.end()) {
                    DLOG("没有找到队列: %s 的管理句柄\n", qname.c_str());
                    return false;
                }
                qcp = it->second;
            }
            return qcp->empty();              
        }

        bool exists(const std::string& qname, const std::string& ctag) {
            QueueConsumer::ptr qcp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _qconsumers.find(qname);
                if (it == _qconsumers.end()) {
                    DLOG("没有找到队列: %s 的管理句柄\n", qname.c_str());
                    return false;
                }
                qcp = it->second;
            }
            return qcp->exists(ctag);   
        }

        void clear() {
            std::unique_lock<std::mutex> lock(_mutex);
            _qconsumers.clear();
        }

        ~ConsumerManager() {
        }
    private:
        std::mutex _mutex;
        std::unordered_map<std::string, QueueConsumer::ptr> _qconsumers;
    };
}

#endif