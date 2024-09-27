#include <iostream>
#include <random>
#include <iomanip>
#include <sstream>
#include <atomic>

class UUIDHelper {
public:
    static std::string uuid() {
        // uuid的数据格式为一个8-4-4-4-12的16进制字符串，如：7a91a05f-52a1-6a01-0000-000000000001
        std::random_device rd;
        // 使用一个机器随机数作为伪随机数的种子
        // 机器数：使用硬件生成的一个数据，生成效率较低
        std::mt19937_64 generator(rd());
        // 生成的数据范围为0~255
        std::uniform_int_distribution<int> distribution(0, 255);
        std::stringstream ss;
        for (int i = 0; i < 8; i++) {
            ss << std::setw(2) << std::setfill('0') << std::hex << distribution(generator);
            if (i == 3 || i == 5 || i == 7)
                ss << "-";
        }
        // std::cout << ss.str() << std::endl;
        static std::atomic<size_t> seq(1);
        size_t num = seq.fetch_add(1);
        for (int i = 7; i >= 0; i--) {
            ss << std::setw(2) << std::setfill('0') << std::hex << (num >> (i * 8));
            if (i == 6)
                ss << "-";
        }     
        return ss.str();   
    }
};

int main() {
    // std::random_device rd;
    // std::mt19937_64 generator(rd());
    // std::uniform_int_distribution<int> distribution(0, 255);
    // std::stringstream ss;
    // for (int i = 0; i < 8; i++) {
    //     ss << std::setw(2) << std::setfill('0') << std::hex << distribution(generator);
    //     if (i == 3 || i == 5 || i == 7)
    //         ss << "-";
    // }
    // // std::cout << ss.str() << std::endl;
    // std::atomic<size_t> seq(1);
    // size_t num = seq.fetch_add(1);
    // for (int i = 7; i >= 0; i--) {
    //     ss << std::setw(2) << std::setfill('0') << std::hex << (num >> (i * 8));
    //     if (i == 6)
    //         ss << "-";
    // }
    // std::cout << ss.str() << std::endl;
    for (int i = 0; i < 20; i++) 
        std::cout << UUIDHelper::uuid() << std::endl;
    return 0;
}