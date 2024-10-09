// #pragma once
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <memory>
#include <future>
#include <atomic>

class threadpool {
private:
    using func_t = std::function<void(void)>;

    // 线程入口函数，不断的从任务池中取出任务进行执行
    void entry() {
        // 线程的处理，循环一直处理
        while (!_stop) {
            std::vector<func_t> temp_taskpool;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                // 等待任务池不为空，或者_stop被置为返回
                _cv.wait(lock, [this](){ return _stop || !_taskpool.empty(); });
                // 取出任务执行
                temp_taskpool.swap(_taskpool);
            }
            for (auto& task : temp_taskpool)
                task();
        }
    }

public:
    threadpool(int thread_count = 1) : _stop(false) {
        for (int i = 0; i < thread_count; i++) 
            _threads.emplace_back(&threadpool::entry, this);
        
    }

    // push函数中传入函数及其对应的参数（可变）
    // 返回一个future对象，由于future对象我们并不知道其类型是啥，所以需要返回值设置为auto
    // push内部将函数封装成一个异步任务（packaged_task），同时使用lambda生成一个可调用对象
    // 然后抛入到任务池中，由线程去执行
    template <typename F, typename ...Args>
    auto push(const F&& func, Args&& ...args) -> std::future<decltype(func(args...))> {
        // 1. 将func封装成packaged_task
        using return_type = decltype(func(args...));
        auto functor = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
        auto ptask = std::make_shared<std::packaged_task<return_type()>>(functor);
        std::future<return_type> fu = ptask->get_future();
        // 2. 将封装好的任务放入到task队列中
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto task = [ptask](){ (*ptask)(); };
            _taskpool.push_back(task);
            // 3. 唤醒一个线程去执行
            _cv.notify_one();
        }
        return fu;
    }

        // 等待所有的线程退出
    void stop() {
        _stop = true;
        // 将所有的线程唤醒
        _cv.notify_all();
        for (auto& th : _threads)
            th.join();
    }

    ~threadpool() {
        if (_stop == false)
            stop();
    }
private:
    std::atomic<bool> _stop;
    std::vector<std::thread> _threads;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::vector<func_t> _taskpool; // 任务池
};

int Add(int x, int y) {
    return x + y;
}

int main() {
    threadpool pool;
    for (int i = 0; i < 10; i++) {
        std::future<int> fu = pool.push(Add, i, i + 3);
        int a = fu.get();
        std::cout << a << std::endl;
    }
    pool.stop();
    return 0;
}