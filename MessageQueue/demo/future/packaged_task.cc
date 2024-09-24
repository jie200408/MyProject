#include <iostream>
#include <thread>
#include <future>
#include <memory>
#include <unistd.h>

int Add(int num1, int num2) {
    std::cout << "<-------------cal------------->" << std::endl;   
    return num1 + num2;
}

int main() {
    auto ptask = std::make_shared<std::packaged_task<int(int, int)>>(Add);
    std::future<int> fu = ptask->get_future();

    std::thread thd([ptask](){
        (*ptask)(10, 30);
    });

    int sum = fu.get();
    std::cout << sum << std::endl;

    thd.join();
    return 0;
}