#ifndef __M_HOST_H__
#define __M_HOST_H__

#include <google/protobuf/map.h>
#include "exchange.hpp"
#include "message.hpp"
#include "queue.hpp"
#include "binding.hpp"

namespace mq {

    class VirtualHost {
    public:
        using ptr = std::shared_ptr<VirtualHost>;

        VirtualHost(const std::string& hname, const std::string& basedir, const std::string& dbfile)
            : _host_name(hname),
              _emp(std::make_shared<ExchangeManager>(dbfile)),
              _mmp(std::make_shared<MessageManager>(basedir)),
              _bmp(std::make_shared<BindingManager>(dbfile)),
              _mqmp(std::make_shared<MsgQueueManager>(dbfile))
        {
            // 遍历队列恢复历史消息
            MsgQueueMapper::MsgQueueMap mqp = _mqmp->allQueue();
            for (auto& q : mqp)
                _mmp->initQueueMessage(q.first);
        }

        MsgQueueMapper::MsgQueueMap allQueue() {
            return _mqmp->allQueue();
        }

        bool declareExchange(const std::string& ename, ExchangeType etype, bool edurable, 
            bool eauto_delete, const google::protobuf::Map<std::string, std::string>& eargs) {
            return _emp->declareExchange(ename, etype, edurable, eauto_delete, eargs);
        }

        void deleteExchange(const std::string& ename) {
            // 删除一个交换机，同时需要删除一个交换机的绑定消息
            _bmp->removeExchangeBindings(ename);
            _emp->deleteExchange(ename);
        }

        bool declareQueue(const std::string& msg_name, bool msg_durable, 
            bool msg_exclusive, bool msg_auto_delete, 
            const google::protobuf::Map<std::string, std::string>& msg_args) {
            // 声明一个队列，现在消息管理中将队列进行初始化
            _mmp->initQueueMessage(msg_name);
            return _mqmp->declareQueue(msg_name, msg_durable, msg_exclusive, msg_auto_delete, msg_args);
        }

        void deleteQueue(const std::string& qname) {
            // 删除一个队列，需要将和队列所有有关的数据都删除掉
            _bmp->removeMsgQueueBindings(qname);
            _mmp->destoryQueueMessahe(qname);
            _mqmp->deleteQueue(qname);
        }

        MsgQueueBindingMap exchangeBindings(const std::string& ename) {
            // 获取交换机的绑定信息
            return _bmp->getExchangeBindings(ename);
        }

        bool basicPublish(const std::string& qname, const BasicProperties* bp, const std::string& body) {
            // 增加一条消息
            MsgQueue::ptr mqp = _mqmp->selectQueue(qname);
            if (mqp.get() == nullptr) {
                DLOG("需要增加消息的队列不存在，队列: %s\n", qname.c_str());
                return false;
            }
            return _mmp->insert(qname, bp, body, mqp->durable);
        }

        void basicAck(const std::string& qname, const std::string& msg_id) {
            return _mmp->ack(qname, msg_id);
        }

        // 获取一个队首消息，用于消费
        MessagePtr basicConsume(const std::string& qname) {          
            return _mmp->front(qname);
        }

        bool bind(const std::string& ename, const std::string& qname, const std::string& key) {
            // 绑定前先找到对应的交换机和队列
            MsgQueue::ptr mqp = _mqmp->selectQueue(qname);
            if (mqp.get() == nullptr) {
                DLOG("当前队列不存在，队列: %s\n", qname.c_str());
                return false;
            }
            Exchange::ptr ep = _emp->selectExchange(ename);
            if (ep.get() == nullptr) {
                DLOG("当前交换机不存在，交换机: %s\n", ename.c_str());
                return false;
            }
            return _bmp->bind(ename, qname, key, ep->durable & mqp->durable);
        }

        void unBind(const std::string& ename, const std::string& qname) {   
            return _bmp->unBind(ename, qname);    
        }

        bool existsExchange(const std::string& ename) {
            return _emp->exists(ename);
        }

        bool existsQueue(const std::string& qname) {
            return _mqmp->exists(qname);
        }

        bool existsBinding(const std::string& ename, const std::string& qname) {
            return _bmp->exists(ename, qname);
        }

        Exchange::ptr selectExchange(const std::string& ename) {
            return _emp->selectExchange(ename);
        }

        void clear() {
            _emp->clear();
            _mmp->clear();
            _bmp->clear();
            _mqmp->clear();
        }

    private:
        std::string _host_name;
        ExchangeManager::ptr _emp;
        MessageManager::ptr _mmp;
        BindingManager::ptr _bmp;
        MsgQueueManager::ptr _mqmp;
    };

}

#endif