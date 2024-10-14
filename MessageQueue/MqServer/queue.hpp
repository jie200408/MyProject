#ifndef __M_QUEUE_H__
#define __M_QUEUE_H__

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
    struct MsgQueue {
        std::string name;                                   // 消息队列的名称
        bool durable;                                       // 持久化标志
        bool exclusive;                                     // 独占标志
        bool auto_delete;                                   // 自动删除标志
        google::protobuf::Map<std::string, std::string> args;  // 其他参数

        using ptr = std::shared_ptr<MsgQueue>;

        MsgQueue() {}

        MsgQueue(const std::string& msg_name, bool msg_durable, 
            bool msg_exclusive, bool msg_auto_delete, 
            const google::protobuf::Map<std::string, std::string>& msg_args)
            : name(msg_name),
              durable(msg_durable),
              exclusive(msg_durable),
              auto_delete(msg_auto_delete),
              args(msg_args)
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

    class MsgQueueMapper {
    private:
        #define SQL_DELETE_MQM "drop table if exists queue_table;"
        #define SQL_INSERT_MQM "insert into queue_table values ('%s', %d, %d, %d, '%s');"
        #define SQL_REMOVE_MQM "delete from queue_table where name="
        #define SQL_CREATE_MQM "create table if not exists queue_table (        \
            name varchar(32) primary key,                                   \
            durable int,                                                    \
            exclusive int,                                                  \
            auto_delete int,                                                \
            args varchar(128)                                               \
        );"
        #define SQL_SELECT_MQM "select name, durable, exclusive, auto_delete, args from queue_table;"
        
        static int selectCallback(void* args, int numcol, char** row, char** fields) {
            MsgQueueMap* queue_map = static_cast<MsgQueueMap*>(args);
            MsgQueue::ptr queue = std::make_shared<MsgQueue>();
            queue->name = row[0];
            queue->durable = (bool)std::stoi(row[1]);
            queue->exclusive = (bool)std::stoi(row[2]);
            queue->auto_delete = (bool)std::stoi(row[3]);
            if (row[4]) queue->setArgs(row[4]);
            queue_map->insert(std::make_pair(queue->name, queue));

            return 0;
        }
    public:
        using MsgQueueMap = std::unordered_map<std::string, MsgQueue::ptr>;

        MsgQueueMapper(const std::string& dbname)
            : _sql_helper(dbname)
        {
            std::string parent_path = FileHelper::parentDirectory(dbname);
            FileHelper::createDirectory(parent_path);
            assert(_sql_helper.open());
            createTable();
        }

        void createTable() {
            int ret = _sql_helper.exec(SQL_CREATE_MQM, nullptr, nullptr);
            if (ret == false) {
                ELOG("表格创建失败\n");
                abort();
            }            
        }

        void removeTable() {
            int ret = _sql_helper.exec(SQL_DELETE_MQM, nullptr, nullptr);
            if (ret == false) {
                ELOG("表格删除失败\n");
                abort();
            }                  
        }

        bool insert(MsgQueue::ptr& msgqueue) {
            char buff[256];
            int n = snprintf(buff, sizeof(buff) - 1, SQL_INSERT_MQM, 
                (char*)msgqueue->name.c_str(),
                msgqueue->durable,
                msgqueue->exclusive,
                msgqueue->auto_delete,
                (char*)msgqueue->getArgs().c_str());
            
            buff[n] = 0;
            std::string sql_insert(buff);
            sql_insert += ";";
            return _sql_helper.exec(sql_insert, nullptr, nullptr);
        }

        void remove(const std::string& name) {
            std::stringstream sql_remove;
            sql_remove << SQL_REMOVE_MQM;
            sql_remove << "'" << name << "'";
            int ret = _sql_helper.exec(sql_remove.str(), nullptr, nullptr);
            if (ret == false)
                ELOG("删除消息队列数据失败\n");
        }

        MsgQueueMap recovery() {
            MsgQueueMap result;
            int ret = _sql_helper.exec(SQL_SELECT_MQM, selectCallback, (void*)(&result));
            return result;
        }
    private:
        SqliteHelper _sql_helper;
    };

    class MsgQueueManager {
    public:
        using ptr = std::shared_ptr<MsgQueueManager>;

        MsgQueueManager(const std::string& dbfile)
            : _mapper(dbfile)
        {
            _msg_queues = _mapper.recovery();
        }

        size_t size() {
            std::unique_lock<std::mutex> lock(_mutex);
            return _msg_queues.size();
        }

        bool exists(const std::string& name) {
            auto it = _msg_queues.find(name);
            if (it == _msg_queues.end())
                return false;
            else
                return true;
        }

        void clear() {
            _mapper.removeTable();
            _msg_queues.clear();
        }

        bool declareQueue(const std::string& msg_name, bool msg_durable, 
            bool msg_exclusive, bool msg_auto_delete, 
            const google::protobuf::Map<std::string, std::string>& msg_args) {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _msg_queues.find(msg_name);
            // 若已经存在，则不用插入
            if (it != _msg_queues.end())
                return true;
            MsgQueue::ptr mqp = std::make_shared<MsgQueue>();
            mqp->name = msg_name;
            mqp->durable = msg_durable;
            mqp->auto_delete = msg_auto_delete;
            mqp->args = msg_args;
            mqp->exclusive = msg_exclusive;
            if (msg_durable == true) {
                int ret = _mapper.insert(mqp);
                if (ret == false)
                    return false;
            }
            _msg_queues[mqp->name] = mqp;
            return true;
        }

        void deleteQueue(const std::string& name) {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _msg_queues.find(name);
            // 若当前队列中已经没有了该队列
            if (it == _msg_queues.end())
                return;
            if (it->second->durable)
                _mapper.remove(name);
            _msg_queues.erase(it->first);
        }

        MsgQueue::ptr selectQueue(const std::string& name) {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _msg_queues.find(name);
            // 若当前队列中已经没有了该队列
            if (it == _msg_queues.end())
                return MsgQueue::ptr();
            return _msg_queues[name];
        }

        MsgQueueMapper::MsgQueueMap allQueue() {
            return _msg_queues;
        }
        
    private:
        std::mutex _mutex;
        MsgQueueMapper _mapper;
        MsgQueueMapper::MsgQueueMap _msg_queues;
    };
}

#endif