#include <iostream>
#include <thread>
#include <future>
#include <unistd.h>

int Add(int num1, int num2) {
    std::cout << "<-------------cal------------->" << std::endl;   
    return num1 + num2;
}

// std::launch::async 为异步执行，只要运行到这一行就会直接开始运行
// std::launch::deferred 只有当调用目标对象的get函数的时候才会运行

int main() {

    std::cout << "<-------------1------------->" << std::endl;
    std::future<int> result = std::async(std::launch::deferred, Add, 10, 20);
    sleep(1);
    std::cout << "<-------------2------------->" << std::endl;
    int answer = result.get();
    std::cout << "<-------------3------------->" << std::endl;
    std::cout << answer << std::endl;
    return 0;
}