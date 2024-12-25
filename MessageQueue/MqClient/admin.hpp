#ifndef __M_ADMIN_H__
#define __M_ADMIN_H__

#include "load.hpp"
#include "../MqCommon/admin.pb.h"

namespace mq {
    class AdminLoad {
    private:
        void menuAdmin() {
            _currentOperation = "menuAdmin";
            printf("************ Admin Menu     ************\n");
            printf("*********    1.用户管理      ********\n");
            printf("*********    2.用户数据管理  ********\n");
            printf("*********    3.系统数据管理  ********\n");
            printf("*********    4.订阅管理      ********\n");
            printf("*********    5.服务器器管理  ********\n");
            printf("*********    6.退出                  ********\n");
        }

        void userManager() {
            // 直接展示当前有哪些用户
            _currentOperation = "userManager";

            // 给出所有用户的信息

        }   

        void userDataManager() {

        } 

        void systemDataManager() {

        }

        void subscribeManager() {

        }

        void commandLine() {
            // [username 当前工作模块]> 
            std::cout << "[" << _username << " " << _currentOperation << "]> ";
            fflush(stdout);
        }

    public:

        AdminLoad(mq::Channel::ptr channel) 
            : _channel(channel),
              _username("admin")
        {}

        // 管理员事件循环
        void adminLoop() {

        }

    private:
        std::string _username;
        std::string _currentOperation;
        std::unordered_map<std::string, std::string> _user_password;
        mq::UserType _user_type;
        
        mq::Channel::ptr _channel;
        
    };
}

#endif