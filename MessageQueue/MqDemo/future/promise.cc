#include <iostream>
#include <thread>
#include <future>
#include <unistd.h>

void Add(int num1, int num2, std::promise<int>& prom) {
    sleep(2);
    prom.set_value(num1 + num2);
}

int main() {
    std::promise<int> prom;
    // 将fu与prom关联起来，才可以实现异步运行的过程中获取到prom中的值的功能
    std::future<int> fu = prom.get_future();
    std::thread thd(Add, 10, 20, std::ref(prom));

    // 若prom中的任务并没有完成，那么将会阻塞在这里
    int answer = fu.get();
    std::cout << answer << std::endl;
    thd.join();
    return 0;
}