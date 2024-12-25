#ifndef __M_BINDING_H__
#define __M_BINDING_H__

#include "../MqCommon/logger.hpp"
#include "../MqCommon/helper.hpp"
#include "../MqCommon/msg.pb.h"
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <sstream>
#include <vector>

namespace mq { 
    class Binding {
    public:
        using ptr = std::shared_ptr<Binding>;

        std::string exchange_name;      // 交换机名称
        std::string msgqueue_name;      // 队列名称
        std::string binding_key;        // 绑定键值

        Binding() {}

        Binding(const std::string& ename, const std::string& qname, const std::string key)
            : exchange_name(ename),
              msgqueue_name(qname),
              binding_key(key)
        {}
    };
    // 一个msgqueue可以找到对应的绑定信息
    using MsgQueueBindingMap = std::unordered_map<std::string, Binding::ptr>;
    // 一个交换机就可以找到对应的所有绑定的队列信息
    using BindingMap = std::unordered_map<std::string, MsgQueueBindingMap>;

    class BindingMapper {
    private:
        // 持久化数据管理sql语句
        #define SQL_DELETE_BM "drop table if exists binding_table;"
        #define SQL_INSERT_BM "insert into binding_table values ('%s', '%s', '%s');"
        #define SQL_REMOVE_BM "delete from binding_table where exchange_name='%s' and msgqueue_name='%s';"
        #define SQL_REMOVE_EXCHANGE_BM "delete from binding_table where exchange_name='%s';"
        #define SQL_REMOVE_MSGQUEUE_BM "delete from binding_table where msgqueue_name='%s';"
        #define SQL_CREATE_BM "create table if not exists binding_table (      \
            exchange_name varchar(32),                                      \
            msgqueue_name varchar(32),                                      \
            binding_key varchar(128)                                        \
        );"
        #define SQL_SELECT_BM "select exchange_name, msgqueue_name, binding_key from binding_table;"

        // 持久化恢复数据持久化回调函数
        static int selectCallback(void* args, int numcol, char** row, char** fields) {
            BindingMap* result = static_cast<BindingMap*>(args);
            Binding::ptr bp = std::make_shared<Binding>(row[0], row[1], row[2]);
            MsgQueueBindingMap& mqbp = (*result)[bp->exchange_name];
            mqbp[bp->msgqueue_name] = bp;
            return 0;
        }

    public:
        BindingMapper(const std::string& dbfile)
            : _sql_helper(dbfile)
        {
            std::string parent_path = FileHelper::parentDirectory(dbfile);
            FileHelper::createDirectory(parent_path);
            assert(_sql_helper.open());
            createTable();
        }

        void createTable() {
            int ret = _sql_helper.exec(SQL_CREATE_BM, nullptr, nullptr);
            if (ret == false) {
                ELOG("表格创建失败\n");
                abort();
            }
        }

        void removeTable() {
            int ret = _sql_helper.exec(SQL_DELETE_BM, nullptr, nullptr);
            if (ret == false) {
                ELOG("表格删除失败\n");
                abort();
            }       
        }

        bool insert(Binding::ptr& binding) {
            char buff[256];
            int n = snprintf(buff, sizeof(buff) - 1, SQL_INSERT_BM, 
                (char*)binding->exchange_name.c_str(), 
                (char*)binding->msgqueue_name.c_str(), 
                (char*)binding->binding_key.c_str());
            buff[n] = 0;
            std::string insert_sql(buff);
            return _sql_helper.exec(insert_sql, nullptr, nullptr);
        }

        void remove(const std::string& ename, const std::string& qname) {
            char buff[256];
            int n = snprintf(buff, sizeof(buff) - 1, SQL_REMOVE_BM, ename.c_str(), qname.c_str());
            buff[n] = 0;
            std::string remove_sql(buff);
            _sql_helper.exec(remove_sql, nullptr, nullptr);
        }

        // 移除绑定关系
        void removeExchangeBindings(const std::string& ename) {
            char buff[256];
            int n = snprintf(buff, sizeof(buff) - 1, SQL_REMOVE_EXCHANGE_BM, ename.c_str());
            buff[n] = 0;
            std::string remove_sql(buff);
            _sql_helper.exec(remove_sql, nullptr, nullptr);        
        }

        void removeMsgQueueBindings(const std::string& qname) {
            char buff[256];
            int n = snprintf(buff, sizeof(buff) - 1, SQL_REMOVE_MSGQUEUE_BM, qname.c_str());
            buff[n] = 0;
            std::string remove_sql(buff);
            _sql_helper.exec(remove_sql, nullptr, nullptr);  
        }

        BindingMap recovery() {
            BindingMap result;
            _sql_helper.exec(SQL_SELECT_BM, selectCallback, (void*)(&result));
            return result;
        }

    private:
        SqliteHelper _sql_helper;
    };

    class BindingManager {
    public:
        using ptr = std::shared_ptr<BindingManager>;

        BindingManager(const std::string& dbfile)
            : _mapper(dbfile)
        {
            _bindings = _mapper.recovery();
        }

        bool bind(const std::string& ename, const std::string& qname, const std::string& key, bool durable) {
            std::unique_lock<std::mutex> lock(_mutex);
            // 需要先检查是否已经绑定
            auto eit = _bindings.find(ename);
            if (eit != _bindings.end() && eit->second.find(qname) != eit->second.end())
                return true;
            // 创建对应的MsgQueueMap
            MsgQueueBindingMap& mqbp = _bindings[ename];
            Binding::ptr bp = std::make_shared<Binding>(ename, qname, key);
            if (durable) {
                bool ret = _mapper.insert(bp);
                if (ret == false)
                    return false;
            }
            mqbp.insert(std::make_pair(qname, bp));
            return true;
        }

        void unBind(const std::string& ename, const std::string& qname) {
            std::unique_lock<std::mutex> lock(_mutex);
            
            // 先查找当前绑定中是否存在这两个信息
            auto eit = _bindings.find(ename);
            if (eit == _bindings.end())
                return;
            auto qit = eit->second.find(qname);
            if (qit == eit->second.end())
                return;
            // 现在删除
            MsgQueueBindingMap& mqbp = _bindings[ename];
            _mapper.remove(ename, qname);
            mqbp.erase(qname);
        }

        void removeExchangeBindings(const std::string& ename) {
            std::unique_lock<std::mutex> lock(_mutex);
            auto eit = _bindings.find(ename);
            if (eit == _bindings.end())
                return;
            // 现在遍历mqbp进行删除
            _mapper.removeExchangeBindings(ename);
            _bindings.erase(ename);
        }

        void removeMsgQueueBindings(const std::string& qname) {
            std::unique_lock<std::mutex> lock(_mutex);
            _mapper.removeMsgQueueBindings(qname);
            // 开始循环交换机映射的绑定
            
            for (auto& eit : _bindings) {
                // 在每个交换机中寻找与其绑定的消息队列
                MsgQueueBindingMap& mqbp = eit.second;
                auto qit = mqbp.find(qname);
                if (qit == mqbp.end())
                    continue;
                // // 删除该对应的绑定信息
                // std::string ename = eit.first;
                // _mapper.remove(ename, qname);
                mqbp.erase(qname);
            }

        }

        Binding::ptr getBinding(const std::string& ename, const std::string& qname) {
            std::unique_lock<std::mutex> lock(_mutex);
            // 先查找当前绑定中是否存在这两个信息
            auto eit = _bindings.find(ename);
            if (eit == _bindings.end())
                return Binding::ptr();
            auto qit = eit->second.find(qname);
            if (qit == eit->second.end())
                return Binding::ptr();
            return qit->second;                  
        }

        MsgQueueBindingMap getExchangeBindings(const std::string& ename) {
            std::unique_lock<std::mutex> lock(_mutex);
            auto eit = _bindings.find(ename);
            if (eit == _bindings.end())
                return MsgQueueBindingMap();
            return eit->second;
        }

        size_t size() {
            std::unique_lock<std::mutex> lock(_mutex);
            size_t total = 0;
            for (auto eit : _bindings)
                total += eit.second.size();
            return total;
        }

        bool exists(const std::string& ename, const std::string& qname) {
            std::unique_lock<std::mutex> lock(_mutex);
            // 先查找当前绑定中是否存在这两个信息
            auto eit = _bindings.find(ename);
            if (eit == _bindings.end())
                return false;
            auto qit = eit->second.find(qname);
            if (qit == eit->second.end())
                return false;
            return true;           
        }

        void clear() {
            std::unique_lock<std::mutex> lock(_mutex);
            _mapper.removeTable();
            _bindings.clear();
        }
    private:
        std::mutex _mutex;
        BindingMapper _mapper;
        BindingMap _bindings;
    };
}

#endif