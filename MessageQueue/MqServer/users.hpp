#ifndef __M_USERS_H__
#define __M_USERS_H__

#include "../MqCommon/user.pb.h"
#include "../MqCommon/helper.hpp"
#include "../MqCommon/logger.hpp"
#include <string>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <memory>
#include <cassert>

namespace mq {
    // using UserLoginPtr = std::shared_ptr<userLogin>;
    // using userRegisterPtr = std::shared_ptr<userRegister>;

    class User {
    public:
        using ptr = std::shared_ptr<User>;
        std::string username;
        std::string password;
        int user_type;

        User() {}

        User(const std::string& uname, const std::string& psword, int type)
            : username(uname),
              password(psword),
              user_type(type)
        {}
    };

    // using UserMap = std::unordered_map<std::string, std::string>;
    using UserMap = std::unordered_map<std::string, std::pair<std::string, int>>;

    class UserMapper {
    private:
        // 对应的持久化sqk语句
        #define SQL_DELETE_USR "drop table if exists user_table;"
        #define SQL_INSERT_USR "insert into user_table values ('%s', '%s', %d);"
        #define SQL_REMOVE_USR "delete from user_table where username="
        #define SQL_CREATE_USR "create table if not exists user_table (     \
            username varchar(32) primary key,                               \
            password varchar(128),                                          \
            usertype int                                                    \
        );"
        #define SQL_SELECT_USR "select username, password, usertype from user_table;"

        static int selectCallback(void* args, int numcol, char** row, char** fields) {
            UserMap* users = static_cast<UserMap*>(args);
            User::ptr up = std::make_shared<User>(row[0], row[1], std::stoi(row[2]));
            // 在user中插入元素
            // users->insert(std::make_pair(up->username, up->password));
            (*users)[up->username] = std::make_pair(up->password, up->user_type);
            return 0;
        }

    public:
        UserMapper(const std::string& dbname)
            : _sql_helper(dbname)
        {
            // 获取父级路径
            std::string parentPath = FileHelper::parentDirectory(dbname);
            // 创建父级路径
            FileHelper::createDirectory(parentPath);
            assert(_sql_helper.open());
            // 创建user数据库表
            this->createTable();
        }

        void createTable() {
            int ret = _sql_helper.exec(SQL_CREATE_USR, nullptr, nullptr);
            if (ret == false) {
                ELOG("表格创建失败\n");
                abort();
            }
        }

        void removeTable() {
            int ret = _sql_helper.exec(SQL_DELETE_USR, nullptr, nullptr);
            if (ret == false) {
                ELOG("表格删除失败\n");
                abort();
            }
        }

        // 主要提供两个接口
        // 一个用户注销，一个用户注册
        bool Register(const User::ptr& user) {
            // 在底层数据库中查找是否存在该用
            
            char buff[256];
            int n = snprintf(buff, sizeof(buff) - 1, SQL_INSERT_USR,
                (char*)user->username.c_str(),
                (char*)user->password.c_str(),
                user->user_type
            );
            buff[n] = 0;
            std::string sql_insert(buff);
            return _sql_helper.exec(sql_insert, nullptr, nullptr);
        } 

        // 用户注销
        void Logout(const std::string& username) {
            std::stringstream sql_remove;
            sql_remove << SQL_REMOVE_USR;
            sql_remove << "'" << username << "';";
            int ret = _sql_helper.exec(sql_remove.str(), nullptr, nullptr);
            if (ret == false) 
                ELOG("注销用户失败\n");
        }

        UserMap recovery() {
            UserMap users;
            int ret = _sql_helper.exec(SQL_SELECT_USR, selectCallback, (void*)(&users));
            return users;
        }
    private:
        SqliteHelper _sql_helper;
    };

    class UserManager {
    public:
        using ptr = std::shared_ptr<UserManager>;

        UserManager(const std::string& dbfile)
            : _mapper(dbfile)
        {
            _users = _mapper.recovery();
            // 查看users中是否存在admin，不存在插入管理员用户
            auto it = _users.find("admin");
            if (it == _users.end()) {
                bool ret = _mapper.Register(std::make_shared<User>("admin", "123456", 0));
                if (ret)
                    _users["admin"] = std::make_pair("123456", 0);
                else
                    ELOG("管理员用户加载失败\n");
            }
        }

        // 用户登陆，用户注销，用户注册
        std::pair<bool, int> login(const std::string& username, const std::string& password) {
            // User user(username, password);
            auto it = _users.find(username);
            if (it == _users.end() || it->second.first != password)
                return std::make_pair(false, -1);
            else
                return std::make_pair(true, it->second.second);
        }

        // 注销
        void logout(const std::string& username) {
            _mapper.Logout(username);
            _users.erase(username);
        }

        // 注册
        bool signIn(const std::string& username, const std::string& password, UserType user_type) {
            auto it = _users.find(username);
            // 用户已经存在
            if (it != _users.end())
                return false;
            // 1表示接收者 2表示推送者
            int type = user_type == mq::RECIVER ? 1 : 2;


            bool ret =  _mapper.Register(std::make_shared<User>(username, password, type));
            if (ret)
                _users[username] = std::make_pair(password, type);
            return ret;
        }

        UserMap allUsers() {
            return _users;
        }

        std::string getAllUsers() {
            // 使用username~~~password%%%usertype~~%%username~~~password%%%usertype
            _users = _mapper.recovery();
            std::string user_psword_sep("~~~");
            std::string psword_type_sep("%%%");
            std::string users_sep("~~%%");
            std::string allUsers;
            for (auto& user : _users) {
                std::string u;
                u += user.first + user_psword_sep;
                u += user.second.first + psword_type_sep;
                u += std::to_string(user.second.second);
                allUsers += u + users_sep;
            }
            ILOG("all users: %s\n", allUsers.c_str());
            return allUsers;
        }
    private:
        UserMapper _mapper;
        UserMap _users;

        // 在用户模块，0表示管理员用户，1表示消息接收用户，2表示消息发布用户
    };

}

#endif