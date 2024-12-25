#ifndef __M_LOAD_H__
#define __M_LOAD_H__

#include "connection.hpp"
#include "channel.hpp"
#include "../MqCommon/admin.pb.h"
#include "../MqCommon/threadpool.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <experimental/filesystem>
#include <iomanip>

#include <termios.h> // Linux: 用于终端控制
#include <unistd.h>

namespace mq {

    namespace fs = std::experimental::filesystem;

    static const std::string basedir = "./data/";
    static const std::string information_sep = "!@#$%";
    static const std::string exchange_sep = "$$$";
    static const std::string queue_sep = "###";
    static const std::string binding_sep = "!!!";
    // binding的存储: binding@@@exchange***queue!!!binding@@@exchange***queue
    static const std::string exchange_queue_sep = "***";
    static const std::string binding_exchange_sep = "@@@";

    static const std::string information_suffix = "/info.txt";
    static const std::string exchange_suffix = "/exchange.txt";
    static const std::string queue_suffix = "/queue.txt";
    static const std::string binding_suffix = "/binding.txt";

    class LoadHelper {
    private:
        std::string getPassword() {
            std::string password;
            struct termios oldAttr, newAttr;

            // 获取当前终端设置
            tcgetattr(fileno(stdin), &oldAttr);
            newAttr = oldAttr;
            // 禁用回显和缓冲模式
            newAttr.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(fileno(stdin), TCSAFLUSH, &newAttr);

            // 读取密码字符
            char c;
            while ((c = getchar()) != '\n') {
                password += c;
                std::cout << '*'; // 实时回显星号
                std::cout.flush();
            }
            std::cout << "\n";

            // 恢复终端设置
            tcsetattr(fileno(stdin), TCSANOW, &oldAttr);
            return password;
        }

        std::pair<bool, mq::UserType> onLoad(const mq::Channel::ptr& channel) {
            std::string username;
            std::string password;
            std::cout << "username:";
            std::cin >> username;
            std::cin.ignore(); // 清除输入缓冲区
            std::cout << "password:";
            password = getPassword();
            UserType user;
            std::pair<bool, mq::UserType> isload = channel->loadPublish(username, password, user);
            if (!isload.first) {
                std::cout << "登陆失败..用户名或密码输入错误...\n";
                return isload;
            }

            // 识别是哪类用户，提供不同的方法
            std::cout << "登陆成功!";
            if (isload.second == mq::PUBLISHER)
                std::cout << "发布者登陆\n";
            else if (isload.second == mq::RECIVER)
                std::cout << "接收者登陆\n";
            else
                std::cout << "管理员登陆\n";
            _user_type = isload.second;
            _username = username;
            return isload;
        }

        std::pair<bool, mq::UserType> Register(const mq::Channel::ptr& channel, std::string& username, std::string& password1, std::string& password2) {
            while (true) {
                std::cout << "Enter username: ";
                std::cin >> username;
                std::cin.ignore(); // 清除输入缓冲区
                std::cout << "Enter password: ";
                password1 = getPassword();
                std::cout << "Re-enter password: ";
                password2 = getPassword();
                if (password1 == password2)
                    break;
                else
                    std::cout << "密码错误, 请重新输入\n";
            }

            std::cout << "选择用户的使用模式(1:消息发布用户, 2:消息接收用户):";
            int userType;
            UserType user_type;
            while (true) {
                std::cin >> userType;
                if (userType == 1) {
                    user_type = mq::PUBLISHER;
                    break;
                } else if (userType == 2) {
                    user_type = mq::RECIVER;
                    break;
                }
                std::cout << "重新选择你的使用模式(1:消息发布用户, 2:消息接收用户):";
            }
            std::pair<bool, mq::UserType> isRegister = channel->registerPublish(username, password1, user_type);
            if (!isRegister.first) {
                std::cout << "注册失败\n";
                return isRegister;
            }
            
            if (isRegister.second == mq::PUBLISHER)
                std::cout << "发布者注册成功\n";
            else
                std::cout << "接收者注册成功\n";            
            return isRegister;
        }

        std::pair<bool, mq::UserType> onRegister(const mq::Channel::ptr& channel) {
            std::string username;
            std::string password1, password2; 

            std::pair<bool, mq::UserType> isRegister = Register(channel, username, password1, password2);

            _user_type = isRegister.second;
            _username = username;
            return isRegister;
        }

        void commandLine() {
            // 输出为蓝色
            std::cout << BLUE << "[" << _username << " " << _currentOperation << "]> " << RESET;
            fflush(stdout);
        }

        void deleteUser(const std::string& username, const UserType& user_type) {
            // 删除原来的用户
            _channel->logoutPublish(username, user_type);
            // 还需要删除原来用户的文件夹
            std::string old_path = basedir + username;
            FileHelper::removeDirectory(old_path);
        }

        void createUser(const std::string& username, const std::string& password, const UserType& user_type) {
            // 注册一个新的
            _channel->registerPublish(username, password, user_type);
            // this->Register(_channel, username, pass)
            // 重新创建文件
            this->createFile(username);
        }

        void changeUserInformation() {
            std::string username, password1, password2;
            while (true) {
                std::cout << "Enter username: ";
                std::cin >> username;
                std::cin.ignore(); // 清除输入缓冲区
                std::cout << "Enter password: ";
                password1 = getPassword();
                std::cout << "Re-enter password: ";
                password2 = getPassword();
                if (password1 == password2)
                    break;
                else
                    std::cout << "密码错误, 请重新输入\n";
            }
            
            this->deleteUser(_username, _user_type);
            this->createUser(username, password1, _user_type);
            _username = username;
            std::cout << "修改成功\n";
        }

        void declareExchange() {
            _currentOperation = "declareExchange";
            std::cout << "输入交换机名称 > ";
            std::string exchange_name;
            std::cin >> exchange_name;
            std::cout << "输入交换类型(1:广播(默认) 2:直连 3:主题) > ";
            mq::ExchangeType exchange_type;
            int etype;
            std::cin >> etype;
            if (etype == 2)
                exchange_type = mq::ExchangeType::DIRECT;
            else if (etype == 3)
                exchange_type = mq::ExchangeType::TOPIC;
            else
                exchange_type = mq::ExchangeType::FANOUT;
            google::protobuf::Map<std::string, std::string> map;
            _channel->declareExchange(exchange_name, exchange_type, true, false, map);
            _exchanges.insert(exchange_name);
        }

        void deleteExchange() {
            if (_exchanges.empty()) {
                std::cout << "当前无交换机可以被删除\n";
                return;
            }
            std::cout << "当前存在的交换机有: ";
            for (auto& ename : _exchanges)
                std::cout << ename << " ";
            std::cout << "\n";
            std::string exchange;
            std::cout << "选择对应的交换机 > ";
            std::cin >> exchange;
            auto it = _exchanges.find(exchange);
            if (it == _exchanges.end()) {
                std::cout << "交换机输入错误\n";
                return;
            }
            _exchanges.erase(it);
            _channel->deleteExchange(exchange);
        }

        void deleteQueue() {
            if (_queues.empty()) {
                std::cout << "当前无队列可以被删除\n";
                return;
            }
            std::cout << "当前存在的队列有: ";
            for (auto& qname : _queues)
                std::cout << qname << " ";
            std::string queue;
            std::cout << "\n";
            std::cout << "选择对应的队列: ";
            std::cin >> queue;
            auto it = _queues.find(queue);
            if (it == _queues.end()) {
                std::cout << "队列输入错误\n";
                return;
            }
            _queues.erase(it);
            _channel->deleteQueue(queue);
        }

        void declareQueue() {
            _currentOperation = "declareQueue";
            std::cout << "输入队列名称> ";
            std::string queue_name;
            std::cin >> queue_name;
            if (_queues.empty()) {
                this->registerQueue(queue_name);
                return;
            } 
            google::protobuf::Map<std::string, std::string> map;
            _channel->declareQueue(queue_name, true, false, false, map);
            _queues.insert(queue_name);
        }

        void queueBind() {
            _currentOperation = "queueBind";
            if (_exchanges.empty() || _queues.empty()) {
                std::cout << "当前无交换机或无队列\n";
                return;
            }
            std::cout << "当前存在的交换机: ";
            for (auto& ename : _exchanges)
                std::cout << ename << " ";
            std::cout << "\n";
            std::cout << "当前存在的队列: ";
            for (auto qname : _queues)
                std::cout << qname << " ";
            std::cout << "\n";

            std::cout << "请选择你要绑定的交换机和队列: ";
            std::string queue, exchange;
            std::cin >> exchange >> queue;
            std::cout << "输入绑定键值:";
            std::string binding;
            std::cin >> binding;
            // ILOG("key:%s\n", binding.c_str());
            bool ret = _channel->queueBind(exchange, queue, binding);
            if (ret == false) {
                std::cout << "绑定失败\n";
                return;
            }
            _bindings[binding] = std::make_pair(exchange, queue);
        }

        void showBindings() {
            for (auto& it : _bindings) {
                std::string ename = it.second.first;
                _channel->getExchangeTypePublish(ename);
                int type = std::stoi(_channel->getArgs());
                if (type == 0) 
                    continue; 
                std::string etype;
                if (type == 1)
                    etype = "直连订阅模式";
                else if (type == 2)
                    etype = "广播订阅模式";
                else
                    etype = "主题订阅模式";
                std::cout << it.first << "  ";
                std::cout << it.second.first << " " << it.second.second << " " << etype << "\n";
                // 获取当前交换机的类型
                // 在这里发送一个请求
            }
        }

        void queueUnBind() {
            if (_bindings.empty()) {
                std::cout << "当前交换机和队列之间无绑定关系\n";
                return;
            }
            std::cout << "当前存在的绑定关系:\n";
            this->showBindings();
            std::cout << "输入需要解绑的绑定关系: ";
            std::string binding;
            std::cin >> binding;
            auto it = _bindings.find(binding);
            if (it == _bindings.end()) {
                std::cout << "绑定关系输入错误\n";
                return;
            }
            std::string ename = it->second.first;
            std::string qname = it->second.second;
            _bindings.erase(it);

            _channel->queueUnBind(ename, qname);
        }

        void bindingsOperation() {
            this->readBinding(_binding_file);
            _currentOperation = "bindingsOperation";
            std::cout << "1:绑定交换机和队列 2:解绑交换机和队列\n";
            this->commandLine();
            int op;
            std::cin >> op;
            if (op == 1)
                this->queueBind();
            else if (op == 2)
                this->queueUnBind();
            else
                std::cout << "输入错误\n";
            this->writeBinding(_binding_file);
        }

        void exchangeOperation() {
            this->readExchange(_exchange_file);
            _currentOperation = "exchangeQperation";
            std::cout << "1:增加交换机 2:删除交换机\n";
            this->commandLine();
            int op;
            std::cin >> op;
            if (op == 1)
                this->declareExchange();
            else if (op == 2)
                this->deleteExchange();
            else
                std::cout << "输入错误\n";
            this->writeExchange(_exchange_file);
        }

        void queueOperation() {
            this->readQueue(_queue_file);
            _currentOperation = "queueOperation";
            std::cout << "1:增加队列 2:删除队列\n";
            this->commandLine();
            int op;
            std::cin >> op;
            if (op == 1)
                this->declareQueue();
            else if (op == 2)
                this->deleteQueue();
            else
                std::cout << "输入错误\n";
            this->writeQueue(_queue_file);
        }

        void basicPublish() {
            _currentOperation = "publishInformation";
            if (_exchanges.empty()) {
                std::cout << "当前用户无交换机, 不可以发送消息\n";
                return;
            }
            if (_information.empty()) {
                std::cout << "当前无信息可被发送\n";
                return;
            }
            std::cout << "当前存在的交换机有: ";
            for (auto& e : _exchanges)
                std::cout << e << " ";
            std::cout << "\n";
            std::cout << "输入你要发送的交换机> ";
            fflush(stdout);
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 清理缓冲区
            std::string exchange;
            std::getline(std::cin, exchange);
            std::vector<std::string> exchanges;
            StrHelper::split(exchange, " ", exchanges);
            // 选择对应的消息
            while (true) {
                std::cout << "当前一共还存在" << _information.size() << "条信息\n";
                int op;
                std::cout << "选择第几条信息发送(-1:退出) > ";
                std::cin >> op;
                if (op == -1) {
                    std::cout << "退出...\n";
                    return;
                }
                if (op > _information.size() || op == 0) {
                    std::cout << "输入错误, 退出\n";
                    return;
                }
                std::cout << "输入该条信息的路由标签 > ";
                std::string routing_key;
                std::cin >> routing_key;
                mq::BasicProperties bp;
                bp.set_id(UUIDHelper::uuid());
                bp.set_routing_key(routing_key);
                bp.set_delivery_mode(mq::DeliveryMode::DURABLE);
                for (auto& ename : exchanges) {
                    auto it = _exchanges.find(ename);
                    if (it == _exchanges.end()) {
                        std::cout << *it << " 该交换机不存在\n";
                        continue;
                    }
                    _channel->basicPublish(ename, &bp, _information[op - 1]);
                }
                std::cout << "第" << op << "条信息已发送!(1:删除 2:保存)\n";
                this->commandLine();
                std::cin >> op;
                if (op == 1) {
                    _information.erase(op - 1 + _information.begin());
                    this->writeInformation(_info_file);
                }
                else if (op == 2)
                    continue;
                else {
                    std::cout << "输入错误, 退出...\n";
                    return;
                
                } 

            }

        }

        void basicEdit() {
            std::string filename = _file_path + "edit_temp.txt";
            std::string command = "nano " + filename;
            system(command.c_str());

            FileHelper file_helpr(filename);
            std::string new_info;
            file_helpr.read(new_info);
            _information.emplace_back(new_info);
            std::cout << "是否保存当前草稿(1:保存, 2:不保存) > ";
            int op;
            std::cin >> op;
            if (op == 1)
                return;
            else if (op == 2)
                FileHelper::createFile(filename);
            else
                std::cout << "输入错误\n";
            this->writeInformation(_info_file);
        }

        void publishInformation() {
            this->readInformation(_info_file);
            _currentOperation = "publishInformation";
            std::cout << "1:发送信息 2:编辑信息\n";
            this->commandLine();
            int op;
            std::cin >> op;
            if (op == 1)
                this->basicPublish();
            else if (op == 2)
                this->basicEdit();
            else
                std::cout << "输入错误, 退出\n";
            this->writeInformation(_info_file);
        }

        void showInformation(const std::string& info_file) {
            int op;
            while (true) {
                if (_information.empty()) {
                    std::cout << "当前信箱已经为空, 退出信箱\n";
                    return;
                }
                std::cout << "当前信箱中一共存在" << _information.size() << "条数据, ";
                std::cout << "您想查看第几条信息(-1:退出):\n";
                this->commandLine();
                
                int options;
                std::cin >> options;
                if (options == -1) {
                    std::cout << "退出查看信箱\n";
                    break;
                } else if (options > _information.size()) {
                    std::cout << "当前没有第" << options << "条信息\n";
                    break;
                }
                std::cout << "当前您正在查看的信息为:\n" << _information[options - 1];
                std::cout << "这条信息已经查看，是否选择删除\n";
                std::cout << "1:删除 2:保存\n";
                this->commandLine();
                std::cin >> op;
                if (op == 1) {
                    _information.erase(_information.begin() + options - 1);
                    this->writeInformation(info_file);
                }
                else if (op == 2)
                    continue;
                else {
                    std::cout << "输入错误\n";
                    break;
                } 
            }
        }

        void reciveInformation() {
            _currentOperation = "reciverInformation";
            if (_information.size() == 0) {
                std::cout << "当前信箱为空\n";
                return;
            }
            std::cout << "当前信箱一共有" << _information.size() << "条信息\n";
            std::cout << "1:查看 2:退出\n";
            int op;
            this->commandLine();
            std::cin >> op;
            if (op == 2) {
                std::cout << "退出查看信箱\n";
                return;
            } else if (op != 1) {
                std::cout << "输入错误\n";
                return;
            }

            this->showInformation(_info_file);
        }

        void menuReciver() {
            _currentOperation = "menuReciver";

            std::cout << GREEN << "***********  Reciver Menu  *************\n" << RESET;
            std::cout << CYAN << "********   1.声明/删除交换机       *******\n" << RESET;
            std::cout << CYAN << "********   2.声明/删除队列         *******\n" << RESET;
            std::cout << CYAN << "********   3.绑定/解绑交换机和队列  *******\n" << RESET;
            std::cout << CYAN << "********   4.查看信箱              *******\n" << RESET;
            std::cout << CYAN << "********   5.修改个人信息          *******\n" << RESET;
            std::cout << GREEN  << "********   6.退出                 ********\n" << RESET;
        }

        void menuPublisher() {
            _currentOperation = "menuPublisher";

            std::cout << GREEN << "************ Publisher Menu   **************\n" << RESET;
            std::cout << CYAN << "*********    1.声明/删除交换机       ********\n" << RESET;
            std::cout << CYAN << "*********    2.声明/删除队列         ********\n" << RESET;
            std::cout << CYAN << "*********    3.绑定/解绑交换机和队列 ********\n" << RESET;
            std::cout << CYAN << "*********    4.发送信息              ********\n" << RESET;
            std::cout << CYAN << "*********    5.修改个人信息          ********\n" << RESET;
            std::cout << GREEN  << "*********    6.退出                  ********\n" << RESET;
        }        

        void menuAdmin() {
            _currentOperation = "menuAdmin";

            std::cout << GREEN << "************ Admin Menu     *********\n" << RESET;
            std::cout << CYAN << "*********    1.用户管理      ********\n" << RESET;
            std::cout << CYAN << "*********    2.用户数据管理  ********\n" << RESET;
            std::cout << CYAN << "*********    3.系统数据管理  ********\n" << RESET;
            std::cout << CYAN << "*********    4.订阅管理      ********\n" << RESET;
            std::cout << GREEN  << "*********    5.退出          ********\n" << RESET;
        }

    private:

        void moveUserData(const std::string& old_path, const std::string& new_path) {
            if (!fs::exists(old_path)) {
                std::cerr << "旧目录不存在: " << old_path << std::endl;
                return;
            }

            if (!fs::exists(new_path))
                fs::create_directories(new_path);

            // 遍历旧目录中的所有文件
            for (const auto& entry : fs::directory_iterator(old_path)) {
                const auto& src_path = entry.path();
                auto dest_path = new_path / src_path.filename();

                // 移动文件到新目录
                fs::rename(src_path, dest_path);
            }  
            fs::remove(old_path);
            // std::cout << "用户数据从 " << old_path << " 成功移动到 " << new_path << std::endl;   
        }

        void showUsers() {
            // 给出所有用户的信息
            std::cout << "当前所有注册的用户有:\n";
            printf("用户名          用户密码        用户类型\n");
            for (auto user : _user_password) {
                if (user.first == "admin")
                    continue;
                std::string usertype = user.second.second == 1 ? "消息接收用户" : "消息发布用户";
                printf("%-8s        %-8s        %-12s\n", user.first.c_str(), 
                    user.second.first.c_str(), usertype.c_str());
            }
        }

        void userManager() {
            // 直接展示当前有哪些用户
            this->getAllUsers();
            _currentOperation = "userManager";
            this->showUsers();
            while (true) {
                std::cout << "对以上用户进行操作(1:修改 2:删除 -1:退出)> ";
                int op;
                std::cin >> op;
                if (op == -1) {
                    std::cout << "退出...\n";
                    return;
                }
                std::cout << "username:";
                std::string username;
                std::cin >> username;
                auto it = _user_password.find(username);
                if (it == _user_password.end()) {
                    std::cout << "用户名输入错误, 请重新操作\n";
                    continue;
                }
                if (op == 1) {
                    std::cout << "现在对" << username << "进行修改...\n";
                    std::cout << "输入" << username << "新的用户名:";
                    std::string new_username, new_password;
                    std::cin >> new_username;
                    std::cout << "password:";
                    std::cin >> new_password;
                    std::cout << "修改" << new_username << "用户类型(1:消息接收者 2:消息发布者)> ";
                    std::cin >> op;
                    if (op != 1 && op != 2) {
                        std::cout << "输入错误..退出...\n";
                        return;
                    }
                    UserType user_type = op == 1 ? UserType::RECIVER : UserType::PUBLISHER;
                    UserType old_user_type = it->second.second == 1 ? UserType::RECIVER : UserType::PUBLISHER;
                    // 删除原来用户，添加新用户
                    // 但是同时需要保存他们的数据
                    std::string old_path = basedir + username;
                    std::string new_path = basedir + new_username;
                    // 将old_path下的所有文件，全都移动到new_path当中
                    this->moveUserData(old_path, new_path);
                    _user_password.erase(username);
                    _user_password[new_username] = {new_password, op};
                    // 删除之后，还需要通过信道在服务器底层进行删除信息
                    _channel->logoutPublish(username, old_user_type);
                    _channel->registerPublish(new_username, new_password, user_type);
                    std::cout << username << "成功被修改为" << new_username << "\n";
                } else if (op == 2) {
                    std::cout << "现在对" << username << "进行删除...\n";
                    int usertype = _user_password[username].second;
                    UserType user_type = usertype == 1 ? UserType::RECIVER : UserType::PUBLISHER;
                    _channel->logoutPublish(username, user_type);
                    _user_password.erase(username);
                    // 还需要删除原来用户的文件夹
                    std::string path = basedir + username;
                    FileHelper::removeDirectory(path);
                    std::cout << username << "及其所有数据删除成功\n";

                } else {
                    std::cout << "输入错误..退出...\n";
                    return;
                }
            }

        }   

        void adminExchangeOperation(const std::string& username) {
            // 读取对应的数据
            int op;
            std::string exchange_file = basedir + username + exchange_suffix;
            this->readExchange(exchange_file);
            std::cout << "当前" << username << "存有的交换机有:\n";
            for (auto &e : _exchanges)
                std::cout << e << " ";
            std::cout << "\n";
            std::cout << "是否选择交换机进行操作(1:否 2:是)> ";
            std::cin >> op;
            if (op == 1)
                return;
            else if (op != 2) {
                std::cout << "输入错误...\n";
                return;
            }
            std::string exchange;
            std::cout << "输入需要删除的交换机> ";
            std::cin >> exchange;
            auto it = _exchanges.find(exchange);
            if (it == _exchanges.end()) {
                std::cout << "输入错误...\n";
                return;
            }
            // 使用信道在服务器删除交换机
            _channel->deleteExchange(exchange);
            // 同时在底层更新对应的数据信息
            _exchanges.erase(it);
            this->writeExchange(exchange_file);
        }

        void adminQueueOperation(const std::string& username) {
            // 读取对应的数据
            int op;
            std::string queue_file = basedir + username + queue_suffix;
            this->readQueue(queue_file);
            std::cout << "当前" << username << "存有的队列有:\n";
            for (auto &q : _queues)
                std::cout << q << " ";
            std::cout << "\n";
            std::cout << "是否选择队列进行操作(1:否 2:是)> ";
            std::cin >> op;
            if (op == 1)
                return;
            else if (op != 2) {
                std::cout << "输入错误...\n";
                return;
            }
            std::string queue;
            std::cout << "输入需要删除的队列> ";
            std::cin >> queue;
            auto it = _queues.find(queue);
            if (it == _queues.end()) {
                std::cout << "输入错误...\n";
                return;
            }
            // 使用信道在服务器删除交换机
            _channel->deleteQueue(queue);
            // 同时在底层更新对应的数据信息
            _queues.erase(it);
            this->writeQueue(queue_file);
        }

        void modifyBinding(const std::string& binding, const std::string& ename, const std::string& qname) {
            _channel->getExchangeTypePublish(ename);
            int type = std::stoi(_channel->getArgs());
            if (type == 0) {
                std::cout << "修改订阅模式失败...\n";
                return;
            }
            std::string etype;
            if (type == 1)
                etype = "直连订阅模式";
            else if (type == 2)
                etype = "广播订阅模式";
            else
                etype = "主题订阅模式";
            std::cout << "当前订阅模式为: " << etype << "\n";
            std::cout << "选择新的订阅模式(1:直连订阅模式 2:广播订阅模式 3:主题订阅模式)\n";
            int op;
            this->commandLine();
            std::cin >> op;
            if (op == type) {
                std::cout << "当前订阅模式正为: " << etype << " ,不用修改\n";
                return;
            }
            ExchangeType exchange_type;
            if (op == 1) 
                exchange_type = ExchangeType::DIRECT;
            else if (op == 2)
                exchange_type = ExchangeType::FANOUT;
            else if (op == 3) 
                exchange_type = ExchangeType::TOPIC;
            else {
                std::cout << "输入错误...退出...\n";
                return;
            }
            // 删除原来的绑定关系 交换机
            _channel->queueUnBind(ename, qname);
            _channel->deleteExchange(ename);
            // 声明新的交换机 新的绑定关系
            google::protobuf::Map<std::string, std::string> map;
            _channel->declareExchange(ename, exchange_type, true, false, map);
            _channel->queueBind(ename, qname, binding);
        }

        void adminBindingOperation(const std::string& username) {
            // 读取对应的数据
            int op;
            std::string binding_file = basedir + username + binding_suffix;
            this->readBinding(binding_file);
            if (_bindings.empty()) {
                std::cout << "当前用户无任何绑定关系\n";
                return;
            }
            std::cout << "当前" << username << "存有的绑定关系有:\n";
            this->showBindings();

            std::cout << "是否选择绑定进行操作(1:否 2:是)> ";
            std::cin >> op;

            if (op == 1) {
                std::cout << "退出...\n";
                return;
            } else if (op != 2) {
                std::cout << "输入错误..退出...\n";
                return;
            }

            std::cout << "选择需要删除的绑定关系> ";

            std::string binding;
            std::cin >> binding;
            auto it = _bindings.find(binding);
            if (it == _bindings.end()) {
                std::cout << "输入错误..退出...\n";
                return;
            }
                // // 使用信道在服务器删除交换机
                _channel->queueUnBind(it->second.first, it->second.second);
                // // 同时在底层更新对应的数据信息
                _bindings.erase(it);
                this->writeBinding(binding_file);
                std::cout << "删除绑定关系成功...\n";
                return;
        }

        void adminSubscribeOperation(const std::string& username) {
            // 读取对应的数据
            int op;
            std::string binding_file = basedir + username + binding_suffix;
            this->readBinding(binding_file);
            if (_bindings.empty()) {
                std::cout << "当前用户无任何绑定关系\n";
                return;
            }
            std::cout << "当前" << username << "存有的绑定关系有:\n";
            this->showBindings();

            std::cout << "选择以上的订阅模式> ";
            std::string binding;
            std::cin >> binding;
            auto it = _bindings.find(binding);
            if (it == _bindings.end()) {
                std::cout << "输入错误..退出...\n";
                return;
            }
            std::cout << "是否选择绑定进行操作(1:删除 2:修改订阅模式 -1:退出)> ";
            std::cin >> op;
            if (op == 1) {
                // // 使用信道在服务器删除交换机
                _channel->queueUnBind(it->second.first, it->second.second);
                // // 同时在底层更新对应的数据信息
                _bindings.erase(it);
                this->writeBinding(binding_file);
                std::cout << "删除绑定关系成功...\n";
                return;
            } else if (op == -1) {
                std::cout << "退出...\n";
                return;
            }
            // 修改订阅模式
            this->modifyBinding(it->first, it->second.first, it->second.second);
        }

        void adminInformationOperation(const std::string& username) {
            // 读取对应的数据
            int op;
            std::string info_file = basedir + username + information_suffix;
            this->readInformation(info_file);
            // this->showBindings();
            this->showInformation(info_file);
        }

        void adminUserOperation() {
            std::cout << "1:增加用户 2:删除用户\n";
            this->commandLine();
            int op;
            std::cin >> op;
            if (op == 1) {
                std::string username, password1, password2;
                std::pair<bool, mq::UserType> isRegister = Register(_channel, username, password1, password2);
                if (!isRegister.first) {
                    return;
                }
                _user_password[username] = {password1, isRegister.second};
                // this->createUser();
                // 在这一步已经创建成功，只需要创建对象的文件夹即可
                this->createFile(username);
            } else if (op == 2) {
                this->showUsers();
                std::string username;
                std::cout << "选择用户> ";
                std::cin >> username;
                auto it = _user_password.find(username);
                if (it == _user_password.end() || it->first == std::string("admin")) {
                    std::cout << "输入错误..退出...\n";
                    return;
                }
                // 已经选中了特定的用户，删除对应的用户
                UserType user_type = it->second.second == 1 ? UserType::RECIVER : UserType::PUBLISHER;
                this->deleteUser(username, user_type);
                _user_password.erase(username);
            } else {
                std::cout << "输入错误..退出...\n";
                return;
            }
        }

        void userDataManager() {
            _currentOperation = "userDataManager";
            int op;
            std::cout << "进行用户数据操作(1:垃圾回收 2:查看/修改用户信箱数据)> ";
            std::cin >> op;
            if (op == 1) {
                // 进行垃圾回收清理系统内部无用的信息
                std::cout << "开始进行垃圾回收...\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                _channel->garbageRecivePublish();
                std::cout << "垃圾回收成功\n";
            } else if (op == 2) {
                // this->adminUserOperation();
                this->showUsers();
                std::cout << "请选择对应的用户> ";
                std::string username;
                std::cin >> username;
                auto it = _user_password.find(username);
                if (it == _user_password.end()) {
                    std::cout << "用户名输入错误..退出...\n";
                    return;
                }
                this->adminInformationOperation(username);
            } else {
                std::cout << "输入错误..退出...\n";
            }
        } 

        void systemDataManager() {
            _currentOperation = "systemDataManager";
            this->showUsers();
            std::cout << "请选择对应的用户> ";
            std::string username;
            std::cin >> username;
            auto it = _user_password.find(username);
            if (it == _user_password.end()) {
                std::cout << "输入错误..退出...\n";
                return;
            }
            while (true) {
                std::cout << "请选择" << username << "的数据信息(1:交换机 2:队列 3:绑定关系 -1:退出)\n";
                this->commandLine();
                int op;
                std::cin >> op;
                // 对于admin中的exchange queue info binding变量而言，只用来作为暂时存储容器
                if (op == 1) {
                    this->adminExchangeOperation(username);
                } else if (op == 2) {
                    this->adminQueueOperation(username);
                } else if (op == 3) {
                    this->adminBindingOperation(username);
                } else if (op == -1) {
                    std::cout << "退出...\n";
                    return;
                } else {
                    std::cout << "输入错误..退出...\n";
                    return;
                }
            }
        }

        void subscribeManager() {
            _currentOperation = "subscribeManager";
            // this->commandLine();
            // 读取所有用户的订阅关系
            for (auto& user : _user_password) {
                if (user.first == std::string("admin"))
                    continue;
                std::string binding_file = basedir + user.first + binding_suffix;
                this->readBinding(binding_file);
                if (_bindings.empty()) {
                    std::cout << "当前" << user.first << "用户无任何订阅\n";
                    continue;
                }
                std::cout << "当前" << user.first << "存有的订阅有:\n";
                this->showBindings();
            }
            std::cout << "是否选择对以上订阅关系进行操作(1:是 2:否):\n";
            int op;
            this->commandLine();
            std::cin >> op;
            if (op == 2) {
                std::cout << "退出...\n";
                return;
            } else if (op != 1) {
                std::cout << "输入错误..退出...\n";
                return;
            }
            std::cout << "选择对应用户> ";
            std::string username;
            std::cin >> username;
            auto it = _user_password.find(username);
            if (it == _user_password.end()) {
                std::cout << "用户不存在..退出...\n";
                return;
            }
            this->adminSubscribeOperation(username);
        }

    private:
        void callBack(const mq::Channel::ptr& channel, const std::string& consumer_tag, const mq::BasicProperties* bp, const std::string& body) {
            // 分隔符使用!@#$%
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _info_count++;
            }
            std::cout << "\n注意注意! 当前收到一条信息!可以通过4来查看信箱\n";
            this->commandLine();
            fflush(stdout);

            _information.emplace_back(body);
        }

        void createFile(const std::string& username) {
            _info_file = basedir + username + information_suffix;
            _queue_file = basedir + username + queue_suffix;
            _exchange_file = basedir + username + exchange_suffix;
            _binding_file = basedir + username + binding_suffix;
            _file_path = basedir + username + "/";

            if (FileHelper(_info_file).exists() == false) {
                std::string path = basedir + username;
                if (FileHelper(path).exists() == false)
                    assert(FileHelper::createDirectory(path));
                assert(FileHelper::createFile(_info_file));
            }

            if (FileHelper(_queue_file).exists() == false)
                assert(FileHelper::createFile(_queue_file));
            if (FileHelper(_exchange_file).exists() == false)
                assert(FileHelper::createFile(_exchange_file));
            if (FileHelper(_binding_file).exists() == false)
                assert(FileHelper::createFile(_binding_file));
        }

        void createFile() {
            this->createFile(_username);
        }

    private:
        void readInformation(const std::string& info_file) {
            std::string allInformation;
            FileHelper file_helper(info_file);
            if (file_helper.read(allInformation) == false) {
                ELOG("读取信箱失败\n");
                return;
            }
            _information.clear();
            StrHelper::split(allInformation, information_sep, _information);
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _info_count = _information.size();
            }
        }

        void readExchange(const std::string& exchange_file_path) {
            std::string allInformation;
            // 读取交换机文件
            _exchanges.clear();
            std::vector<std::string> exchanges;
            FileHelper exchange_file(exchange_file_path);
            if (exchange_file.read(allInformation) == false) {
                ELOG("读取队列失败\n");
                return;
            }
            StrHelper::split(allInformation, exchange_sep, exchanges);
            for (auto& e : exchanges)
                _exchanges.insert(e);
        }

        void readQueue(const std::string& queue_file_path) {
            std::string allInformation;
            // 读取队列文件
            _queues.clear();
            std::vector<std::string> queues;
            FileHelper queue_file(queue_file_path);
            if (queue_file.read(allInformation) == false) {
                ELOG("读取队列失败\n");
                return;
            }
            StrHelper::split(allInformation, queue_sep, queues);
            for (auto& q : queues) {
                if (_queues.empty()) {
                    this->registerQueue(q);
                    return;
                } 
                _queues.insert(q);
            }
                
        }

        void readBinding(const std::string& binding_file_path) {
            std::string allInformation;
            std::vector<std::string> bindings;
            FileHelper binding_file(binding_file_path);
            allInformation.clear();
            if (binding_file.read(allInformation) == false) {
                ELOG("读取队列失败\n");
                return;
            }
            _bindings.clear();
            StrHelper::split(allInformation, binding_sep, bindings);
            for (auto& b : bindings) {
                // binding的存储: binding@@@exchange***queue!!!binding@@@exchange***queue
                std::vector<std::string> b_eqv;
                StrHelper::split(b, binding_exchange_sep, b_eqv);
                std::string binding(b_eqv.front());
                std::string eq(b_eqv.back());
                std::vector<std::string> eqv;
                StrHelper::split(eq, exchange_queue_sep, eqv);
                std::string ename(eqv.front());
                std::string qname(eqv.back());
                _bindings[binding] = {ename, qname};
            }
        }

        void readAllInformation() {
            this->readInformation(_info_file);
            this->readExchange(_exchange_file);
            this->readQueue(_queue_file);
            this->readBinding(_binding_file);
        }

private:
        void writeQueue(const std::string& queue_file) {
            FileHelper::createFile(queue_file);
            FileHelper queue_file_helper(queue_file);
            std::string allQueues;
            for (auto& q : _queues)
                allQueues += q + queue_sep;
            queue_file_helper.write(allQueues);
        }

        void writeExchange(const std::string& exchange_file) {
            FileHelper::createFile(exchange_file);
            std::string allExchanges;
            FileHelper exchange_file_helper(exchange_file);
            for (auto& e : _exchanges)
                allExchanges += e + exchange_sep;
            exchange_file_helper.write(allExchanges);
        }

        void writeBinding(const std::string& binding_file) {
            FileHelper::createFile(binding_file);
            FileHelper binding_file_helper(binding_file);
            std::string allBindings;
            for (auto& b : _bindings) {
                std::string binding;
                binding += b.first + binding_exchange_sep;
                binding += b.second.first + exchange_queue_sep;
                binding += b.second.second;
                allBindings += binding + binding_sep;
            }
            binding_file_helper.write(allBindings);
        }

        void writeInformation(const std::string& info_file) {
            FileHelper::createFile(info_file);
            std::string allInformation;
            FileHelper info_file_helper(info_file);
            for (auto& info : _information) 
                allInformation += info + information_sep;
            info_file_helper.write(allInformation);
        }

        void writeAllInformation() {
            this->writeInformation(_info_file);
            this->writeBinding(_binding_file);
            this->writeQueue(_queue_file);
            this->writeExchange(_exchange_file);
        }

    private:
        void registerQueue(const std::string& qname) {
            google::protobuf::Map<std::string, std::string> googleMap;
            _channel->declareQueue(qname, true, false, false, googleMap);
            _queues.insert(qname);
            _threads.push([this, qname]() {
                this->_channel->basicConsume(this->_username, qname, false, this->_callback);
            });
        }

        void initialReciver() {
            this->createFile();
            _callback = std::bind(&LoadHelper::callBack, this, _channel, std::placeholders::_1, 
                std::placeholders::_2, std::placeholders::_3);
            // 之后在这里改一下
            // google::protobuf::Map<std::string, std::string> googleMap;
            // _channel->declareQueue(_queuename, true, false, false, googleMap);
            // _queues.insert(_queuename);
            // _threads.push([this]() {
            //     this->_channel->basicConsume(this->_username, this->_queuename, false, this->_callback);
            // });
            this->readAllInformation();
        }

        void initialPublisher() {
            this->createFile();
            this->readAllInformation();
        }    

        void getAllUsers() {
            // 使用username~~~password%%%usertype~~%%username~~~password%%%usertype
            std::string user_psword_sep("~~~");
            std::string psword_type_sep("%%%");
            std::string users_sep("~~%%");

            _user_password.clear();

            // 发送一个请求
            _channel->getAllUsersPublish();
            // 现在接收一个请求
            std::string user_password;
            user_password = _channel->getArgs();
            // 现在对user_password字符串进行分割
            // 按照密码
            std::vector<std::string> allUsers;
            StrHelper::split(user_password, users_sep, allUsers);
            for (auto& user : allUsers) {
                std::vector<std::string> u;
                std::vector<std::string> type;
                StrHelper::split(user, user_psword_sep, u);
                StrHelper::split(u.back(), psword_type_sep, type);
                _user_password[u.front()] = {type.front(), std::stoi(type.back())};
                //ILOG("用户名:%s 密码:%s 用户类型:%s\n", u.front().c_str(), type.front().c_str(), type.back().c_str());
            }
        }

        void initialAdmin() {
            this->createFile();
            this->getAllUsers();
            // 当前已经获取到所有的信息
            
            this->readAllInformation();
        }
    public:

        LoadHelper(mq::Channel::ptr channel, const char* qname)
            : _channel(channel),
              _queuename(qname),
              _info_count(0)
        {
        }

        std::pair<bool, mq::UserType> userLoad() {
            int isload = 0;
            std::cout << "选择你的登陆方式(登陆为1, 注册为2):";
            std::cin >> isload;
            while (true) {
                if (isload == 1)
                    return this->onLoad(_channel);
                else if (isload == 2)
                    return this->onRegister(_channel);
                std::cout << "选择你的登陆方式(登陆为1, 注册为2):";
                std::cin >> isload;
            }
        }

        // 信息接收端事件循环
        void reciverLoop() {
            this->initialReciver();
            while (true) {
                this->menuReciver();
                int op;
                this->commandLine();
                std::cin >> op;
                if (op == 5)
                    this->changeUserInformation();
                else if (op == 1)
                    this->exchangeOperation();
                else if (op == 2)
                    this->queueOperation();
                else if (op == 3)
                    this->bindingsOperation();
                else if (op == 4)
                    this->reciveInformation();
                else if (op == 6) {
                    std::cout << "退出...\n";
                    break;
                } else {
                    std::cout << "输入错误\n";
                }
            }
        }

        // 信息发送端事件循环
        void publisherLoop() {
            this->initialPublisher();
            while (true) {
                this->menuPublisher();
                int op;
                this->commandLine();
                std::cin >> op;
                if (op == 5)
                    this->changeUserInformation();
                else if (op == 1)
                    this->exchangeOperation();
                else if (op == 2)
                    this->queueOperation();
                else if (op == 3)
                    this->bindingsOperation();
                else if (op == 4)
                    this->publishInformation();
                else if (op == 6) {
                    std::cout << "退出...\n";
                    break;
                } else {
                    std::cout << "输入错误\n";
                }
            }
        }

        // 管理员事件循环
        void adminLoop() {
            this->initialAdmin();
            while (true) {
                this->menuAdmin();
                int op;
                this->commandLine();
                std::cin >> op;
                if (op == 1) 
                    this->userManager();
                else if (op == 2)
                    this->userDataManager();
                else if (op == 3)
                    this->systemDataManager();
                else if (op == 4)
                    this->subscribeManager();
                else if (op == 5) {
                    std::cout << "退出...\n";
                    break;
                } else {
                    std::cout << "输入错误\n";
                }
            }
        }

        // 需要在本地存储用户的信息，同样使用数据库进行存储，使用用户名存储

        ~LoadHelper() {
            // this->writeAllInformation();
        }

    private:
        mq::Channel::ptr _channel;
        mq::UserType _user_type;
        std::string _username;
        std::string _queuename;
        std::string _currentOperation;
        std::unordered_set<std::string> _exchanges;
        std::unordered_set<std::string> _queues;
        std::unordered_map<std::string, std::pair<std::string, std::string>> _bindings;

        std::vector<std::string> _information;
        size_t _info_count;


        ConsumerCallback _callback;

        threadpool _threads;
        std::string _info_file;
        std::string _queue_file;
        std::string _exchange_file;
        std::string _binding_file;
        std::string _file_path;

        std::mutex _mtx;

        std::unordered_map<std::string, std::pair<std::string, int>> _user_password;
    };

}

// 现在开始测试，检测我们登陆设置
// 检测数据库中的用户

#endif