#ifndef __M_WORKER_H__
#define __M_WORKER_H__

#include <muduo/net/EventLoopThread.h>
#include "../MqCommon/helper.hpp"
#include "../MqCommon/logger.hpp"
#include "../MqCommon/threadpool.hpp"

namespace mq {
    class AsyncWorker {
    // 这一个对象可以用于多个连接使用
    public:
        using ptr = std::shared_ptr<AsyncWorker>;

        threadpool pool;                            // 事件处理线程池
        muduo::net::EventLoopThread loopthread;     // 用于循环监控io事件
    };
}

#endif