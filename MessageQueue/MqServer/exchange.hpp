#ifndef __M_EXCHANGE_H__
#define __M_EXCHANGE_H__

#include "../MqCommon/logger.hpp"
#include "../MqCommon/helper.hpp"
#include "../MqCommon/msg.pb.h"
#include <google/protobuf/map.h>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <sstream>
#include <vector>

namespace mq {
    // 交换机类
    struct Exchange {
        using ptr = std::shared_ptr<Exchange>;

        std::string name;                                      // 交换机名称
        ExchangeType type;                                     // 交换机类型
        bool durable;                                          // 是否继续持久化管理
        bool auto_delete;                                      // 是否自动删除
        google::protobuf::Map<std::string, std::string> args;  // 交换机的其他参数

        Exchange() {}

        Exchange(const std::string& ename, ExchangeType etype, 
            bool edurable, 
            bool eauto_delete, 
            const google::protobuf::Map<std::string, std::string>& eargs)
            : name(ename),
              type(etype),
              durable(edurable),
              auto_delete(eauto_delete),
              args(eargs)
        {}

        // 设置args中的参数，类型为："key=value&key=value...."，相当于将数据反序列化
        void setArgs(const std::string& args_str) {
            if (args_str.empty()) 
                return;
            std::vector<std::string> kvs;
            StrHelper::split(args_str, "&", kvs);
            for (auto& kv : kvs) {
                // 现在将key，value放入到map中
                size_t pos = kv.find('=');
                std::string key = kv.substr(0, pos);
                std::string value = kv.substr(pos + 1);
                args[key] = value;
            }
        }

        // 将数据序列化
        std::string getArgs() {
            std::string result;
            for (auto& it : args) {
                std::string kv = it.first + "=" + it.second + "&";
                result += kv;
            }
            return result;
        }
    };

    // 交换机持久化管理类，数据存储在sqlite数据库中
    class ExchangeMapper {
    private:
        #define SQL_DELETE_EM "drop table if exists exchange_table;"
        #define SQL_INSERT_EM "insert into exchange_table values ('%s', %d, %d, %d, '%s');"
        #define SQL_REMOVE_EM "delete from exchange_table where name="
        #define SQL_CREATE_EM "create table if not exists exchange_table (     \
            name varchar(32) primary key,                                   \
            type int,                                                       \
            durable int,                                                    \
            auto_delete int,                                                \
            args varchar(128)                                               \
        );"
        #define SQL_SELECT_EM "select name, type, durable, auto_delete, args from exchange_table;"

        
        static int selectCallback(void* args, int numcol, char** row, char** fields) {
            ExchangeMap* exchange_map = static_cast<ExchangeMap*>(args);
            Exchange::ptr exchange = std::make_shared<Exchange>();
            exchange->name = row[0];
            exchange->type = (ExchangeType)std::stoi(row[1]);
            exchange->durable = (bool)std::stoi(row[2]);
            exchange->auto_delete = (bool)std::stoi(row[3]);
            if (row[4]) exchange->setArgs(row[4]);
            exchange_map->insert(std::make_pair(exchange->name, exchange));

            return 0;
        }
    public:
        using ExchangeMap = std::unordered_map<std::string, Exchange::ptr>;

        ExchangeMapper(const std::string& dbfile)
            : _sql_helper(dbfile)
        {
            // 先获取dbfile上级文件路径
            std::string parent_path = FileHelper::parentDirectory(dbfile);
            FileHelper::createDirectory(parent_path);
            // 将数据库打开
            assert(_sql_helper.open());
            createTable();
        }

        void createTable() {
            int ret = _sql_helper.exec(SQL_CREATE_EM, nullptr, nullptr);
            if (ret == false) {
                ELOG("表格创建失败\n");
                abort();
            }
        }
        
        void removeTable() {
            // 删除表格
            int ret = _sql_helper.exec(SQL_DELETE_EM, nullptr, nullptr);
            if (ret == false) {
                ELOG("表格删除失败\n");
                abort();
            }            
        }

        bool insert(const Exchange::ptr& exchange) {
            char buff[256];
            int n = snprintf(buff, sizeof(buff) - 1, SQL_INSERT_EM, 
                (char*)exchange->name.c_str(),
                exchange->type,
                exchange->durable,
                exchange->auto_delete,
                (char*)exchange->getArgs().c_str()
            );
            buff[n] = 0;
            std::string cmd(buff);
            cmd += ";";
            return _sql_helper.exec(cmd, nullptr, nullptr);
        }

        void remove(const std::string& name) {
            std::stringstream ss;
            ss << SQL_REMOVE_EM << "'" << name << "'" << ";";
            int ret = _sql_helper.exec(ss.str(), nullptr, nullptr);
            if (ret == false) 
                ELOG("删除交换机数据失败\n");            
        }

        ExchangeMap recovery() {
            ExchangeMap result;
            int ret = _sql_helper.exec(SQL_SELECT_EM, selectCallback, (void*)(&result));
            return result;
        }
    private:
        SqliteHelper _sql_helper;
    };

    // 交换机管理类
    class ExchangeManager {
    public:
        using ptr = std::shared_ptr<ExchangeManager>;

        ExchangeManager(const std::string& dbfile)
            : _mapper(dbfile)
        {
            // 从底层数据库中直接恢复数据
            _exchanges = _mapper.recovery();
        }

        // 声明交换机，增加一个交换机
        bool declareExchange(const std::string& name, ExchangeType type, bool durable, 
            bool auto_delete, const google::protobuf::Map<std::string, std::string>& args) {
            // 需要先加锁
            std::unique_lock<std::mutex> lock(_mutex);
            // 判断当前需要插入的数据是否已经存在，若存在则我们不需要插入
            auto it = _exchanges.find(name);
            if (it != _exchanges.end())
                return true;
            Exchange::ptr exchange = std::make_shared<Exchange>(name, type, durable, auto_delete, args);
            // 判断当前的数据是否需要持久化处理
            if (durable == true) {
                int ret = _mapper.insert(exchange);
                if (ret == false)
                    return false;
            }
            _exchanges[name] = exchange;
            return true;
        }

        // 查找一个交换机
        Exchange::ptr selectExchange(const std::string& name) {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _exchanges.find(name);
            // 当没有找到时直接返回null
            if (it == _exchanges.end())
                return std::make_shared<Exchange>();
            return it->second;
        }

        // 删除交换机
        void deleteExchange(const std::string& name) {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _exchanges.find(name);
            // 若当前的数据中不存在对应的交换机的时候，直接返回
            if (it == _exchanges.end())
                return;
            // 删除数据需要删除交换机存储和持久化交换机管理
            if (it->second->durable)
                _mapper.remove(it->first);
            _exchanges.erase(it->first);
        }

        bool exists(const std::string& name) {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _exchanges.find(name);
            if (it == _exchanges.end())
                return false;
            else    
                return true;
        }

        size_t size() {
            std::unique_lock<std::mutex> lock(_mutex);
            return _exchanges.size();
        }

        void clear() {
            std::unique_lock<std::mutex> lock(_mutex);
            // 删除持久化数据管理中的表格
            _mapper.removeTable();
            _exchanges.clear();
        }
    private:
        std::mutex _mutex;                                          // 防止出现线程安全问题
        ExchangeMapper _mapper;                                     // 交换机持久化数据管理
        std::unordered_map<std::string, Exchange::ptr> _exchanges;  // 所有的交换机及其对应的名称
    };
}

#endif