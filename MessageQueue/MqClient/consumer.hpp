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

namespace mq {
    using ConsumerCallback = std::function<void(const std::string&, const BasicProperties* bp, const std::string&)>;

    struct Consumer {
        using ptr = std::shared_ptr<Consumer>;

        std::string tag;            // 消费者标识           ----> 标识订阅的消费者
        std::string qname;          // 消费者绑定队列名称   ---->  标识订阅者对应的队列
        bool auto_ack;              // 是否自动确认         ----> 对于队列发送过来的消息是否自动确认
        ConsumerCallback callback;  // 回调函数            ---->  获取消息之后处理的回调函数

        Consumer() {}

        Consumer(const std::string& ctag, const std::string& queue_name, bool ack, const ConsumerCallback& cb)
            : tag(ctag),
              qname(queue_name),
              auto_ack(ack),
              callback(cb)
        {}
    };

}

#endif 