#include <iostream>
#include "contacts.pb.h"

int main() {
    contacts::contact conn;
    conn.set_name("小张");
    conn.set_score(90.8);
    conn.set_sn(2);
    std::string str = conn.SerializeAsString();
    

    contacts::contact stu;
    bool res = stu.ParseFromString(str);
    if (res == false) {
        std::cout << "反序列化失败" << std::endl;
        return -1;
    }
    std::cout << stu.name() << std::endl;
    std::cout << stu.sn() << std::endl;
    std::cout << stu.score() << std::endl;
    return 0;
}