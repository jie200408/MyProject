#ifndef __M_MESSAGE_H__
#define __M_MESSAGE_H__

#include "../MqCommon/logger.hpp"
#include "../MqCommon/helper.hpp"
#include "../MqCommon/msg.pb.h"
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <list>

namespace mq {
    #define DATAFILE_SUBFIX ".mqd"
    #define TEMPFILE_SUBFIX ".mqd.tmp"

    using MessagePtr = std::shared_ptr<mq::Message>;

    class MessageMapper {
    private:
        bool insert(MessagePtr& msg, const std::string& filename) {
            FileHelper helper(filename);
            // 将数据序列化
            std::string body = msg->payload().SerializeAsString();
            size_t offset;
            offset = helper.size();
            // 将数据存放到适当的位置
            bool ret = helper.write(body.c_str(), offset, body.size());
            if (ret == false) {
                ELOG("写入数据失败\n");
                return false;
            }
            // 更新数据中存储的位置，以及数据的长度
            msg->set_offset(offset);
            msg->set_length(body.size());
            return true;
        }

        bool load_data(std::list<MessagePtr>& result) {
            // 先将文件打开，然后逐一从文件中取出数据
            FileHelper helper(_datafile);
            size_t offset = 0;
            size_t filesize = helper.size();
            while (offset < filesize) {
                // 从文件中读出数据
                bool ret;
                size_t length = 0;
                ret = helper.read((char*)(&length), offset, 4);
                if (ret == false) {
                    ELOG("读取数据长度失败\n");
                    return false;
                }
                offset += 4;
                std::string body(length, '\0');
                ret = helper.read((char*)(&body[0]), offset, length);
                if (ret == false) {
                    ELOG("读取数据失败\n");
                    return false;
                }
                MessagePtr msgp = std::make_shared<Message>();
                msgp->ParseFromString(body);
                offset += length;
                if (msgp->payload().vaild() == "0")
                    continue;
                result.emplace_back(msgp);
            }
            return true;
        }

        bool createMsgFile() {
            bool ret = FileHelper::createFile(_datafile);
            if (ret == false) {
                ELOG("创建 %s 文件失败\n", _datafile.c_str());
                return false;
            }
            return true;
        }

    public:
        MessageMapper(const std::string& databasedir, const std::string& qname)
            : _qname(qname)
        {
            std::string basedir(databasedir);
            if (basedir.back() != '/') 
                basedir += '/';
            _datafile = basedir + _qname + DATAFILE_SUBFIX;
            _tempfile = basedir + _qname + TEMPFILE_SUBFIX;
            assert(FileHelper::createDirectory(basedir));
            assert(this->createMsgFile());
        }

        bool removeMsgFile() {
            bool ret = FileHelper::removeFile(_datafile);
            if (ret == false) {
                ELOG("移除 %s 文件失败\n", _datafile.c_str());
                return false;
            }
            return true;            
        }

        bool insert(MessagePtr& msg) {
            return this->insert(msg, _datafile);
        }

        bool remove(MessagePtr& msg) {
            // 将数据从文件中删除，删除数据就是将数据的有效位置设置为"0"
            msg->mutable_payload()->set_vaild("0");
            std::string body = msg->payload().SerializeAsString();
            if (body.size() != msg->length()) {
                ELOG("删除数据失败\n");
                return false;
            }
            // 现在将数据重新写入文件原来的位置
            FileHelper helper(_datafile);
            bool ret = helper.write(body.c_str(), msg->offset(), body.size());
            if (ret == false) {
                ELOG("覆盖原来数据失败\n");
                return false;
            }
            return true;
        }

        // 对我们的数据进行垃圾回收
        std::list<MessagePtr> gc() {
            // 1. 先加载数据，拿到有效的数据
            std::list<MessagePtr> result;
            bool ret = this->load_data(result);
            if (ret == false) {
                ELOG("加载数据失败\n");
                return result;
            }
            // 2. 将有效的数据存放到临时文件中
            for (auto& msg : result) {
                ret = this->insert(msg, _tempfile);
                if (ret == false) {
                    ELOG("插入数据失败\n");
                    return result;
                }
            }
            // 3. 删除源文件
            ret = FileHelper::removeFile(_datafile);
            if (ret == false) {
                ELOG("删除源文件失败\n");
                return result;
            }
            // 4. 临时文件重命名
            FileHelper helper(_tempfile);
            ret = helper.rename(_datafile);
            if (ret == false) 
                ELOG("重命名临时文件失败\n");
              
            return result;
        }

    private:
        std::string _qname;
        std::string _datafile;
        std::string _tempfile;
    };

    class QueueMessage {
    private:
        bool gcCheck() {
            // 当数据达到两千条，并且有效数据低于50%的时候需要进行垃圾回收
            if (_total_count > 2000 && _valid_count * 10 / _total_count < 5)
                return true;
            else
                return false;
        }

        void gc() {
            if (this->gcCheck() == false) 
                return;
            // 现在开始进行垃圾回收
            std::list<MessagePtr> msgs = _mapper.gc();

            for (auto& msg : msgs) {
                auto it = _durable_msgs.find(msg->payload().properties().id());
                if (it == _durable_msgs.end()) {
                    ELOG("有一条消息没有被持久化管理\n");
                    msgs.push_back(msg);
                    _durable_msgs[msg->payload().properties().id()] = msg;
                    continue;
                }
                // 需要重新设置消息的偏移量和长度
                it->second->set_offset(msg->offset());
                it->second->set_length(msg->length());
            }
            _valid_count = msgs.size();
            _total_count = msgs.size();
        }

    public:
        using ptr = std::shared_ptr<QueueMessage>;

        QueueMessage(const std::string& basedir, const std::string& qname)
            : _qname(qname),
              _mapper(basedir, qname),
              _valid_count(0),
              _total_count(0)
        {}

        void recovery() {
            // 恢复消息
            std::unique_lock<std::mutex> lock(_mutex);
            _msgs = _mapper.gc();
            for (auto& msg : _msgs) 
                _durable_msgs.insert(std::make_pair(msg->payload().properties().id(), msg));
            _valid_count = _msgs.size();
            _total_count = _msgs.size();
        }

        bool insert(const BasicProperties* bp, const std::string& body, DeliveryMode deliverymode) {
            // 1.先构造对应的消息
            MessagePtr msg = std::make_shared<Message>();
            if (bp == nullptr) {
                msg->mutable_payload()->mutable_properties()->set_id(UUIDHelper::uuid());
                msg->mutable_payload()->mutable_properties()->set_delivery_mode(deliverymode);
                msg->mutable_payload()->mutable_properties()->set_routing_key(std::string());
            } else {
                msg->mutable_payload()->mutable_properties()->set_id(bp->id());
                msg->mutable_payload()->mutable_properties()->set_delivery_mode(bp->delivery_mode());
                msg->mutable_payload()->mutable_properties()->set_routing_key(bp->routing_key());                
            }
            std::unique_lock<std::mutex> lock(_mutex);
            // 2.判断消息是否需要持久化
            if (msg->payload().properties().delivery_mode() == DeliveryMode::DURABLE) {
                // 将消息进行持久化处理
                msg->mutable_payload()->set_vaild("1");
                bool ret = _mapper.insert(msg);
                if (ret == false) {
                    DLOG("持久化 %s 数据失败\n", body.c_str());
                    return false;
                }
                // 持久化数据加一
                _valid_count++;
                _total_count++;
                _durable_msgs[msg->payload().properties().id()] = msg;
            }
            // 3.将消息加入到待推送链表中
            _msgs.push_back(msg);

            return true;
        }

        MessagePtr front() {
            // 拿出队首消息，然后发送出去
            std::unique_lock<std::mutex> lock(_mutex);
            if (_msgs.size() == 0)
                return MessagePtr();
            MessagePtr msg = _msgs.front();
            _msgs.pop_front();
            // 现在将拿出的队首消息加入到待确认消息
            _waitack_msgs[msg->payload().properties().id()] = msg;
            return msg;
        }

        bool remove(const std::string& msg_id) {
            // 删除数据是从待确认消息中进行删除
            std::unique_lock<std::mutex> lock(_mutex);
            // 1.先查找该数据是否存在
            auto it = _waitack_msgs.find(msg_id);
            if (it == _waitack_msgs.end()) {
                ILOG("没有找到需要删除的消息 %s\n", msg_id.c_str());
                return true;
            }
            // 2.判断拿到的消息是否存在持久化性质
            if (it->second->payload().properties().delivery_mode() == DeliveryMode::DURABLE) {
                // 还要将持久化的数据给删去
                _mapper.remove(it->second);
                _durable_msgs.erase(it->second->payload().properties().id());
                // 持久化文件中的有效数据减一
                _valid_count--;
                this->gc();
            }
            // 3.同时删除内存中的数据
            _waitack_msgs.erase(it->second->payload().properties().id());
            return true;
        }

        size_t getable_count() {
            // 可获取到的数据
            std::unique_lock<std::mutex> lock(_mutex);
            return _msgs.size();
        }

        size_t total_count() {
            std::unique_lock<std::mutex> lock(_mutex);
            return _total_count;
        }

        size_t waitack_count() {
            std::unique_lock<std::mutex> lock(_mutex);
            return _waitack_msgs.size();
        }

        size_t durable_count() {
            std::unique_lock<std::mutex> lock(_mutex);
            return _durable_msgs.size();
        }

        void clear() {
            _mapper.removeMsgFile();
            _msgs.clear();
            _durable_msgs.clear();
            _waitack_msgs.clear();
            _valid_count = 0;
            _total_count = 0;
        }
    private:
        std::mutex _mutex;
        std::string _qname;                                             // 队列名称
        MessageMapper _mapper;                                          // 持久化操作句柄
        size_t _valid_count;                                            // 有效消息数量
        size_t _total_count;                                            // 总消息数量
        std::list<MessagePtr> _msgs;                                    // 待推送消息链表
        std::unordered_map<std::string, MessagePtr> _durable_msgs;      // 待持久消息hash
        std::unordered_map<std::string, MessagePtr> _waitack_msgs;      // 待确认消息hash
    };

    class MessageManager {
    public:
        MessageManager(const std::string& basedir)
            : _basedir(basedir)
        {}

        // 初始化队列消息
        void initQueueMessage(const std::string& qname) {
            QueueMessage::ptr msgp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                // 查找当前hashmap中是否已经存在
                auto it = _queue_msgs.find(qname);
                if (it != _queue_msgs.end()) {
                    ILOG("当前队列消息已存在: %s\n", qname.c_str());
                    return;
                }
                msgp = std::make_shared<QueueMessage>(_basedir, qname);
            }
            // 恢复内存中的数据
            msgp->recovery();
        }

        void destoryQueueMessahe(const std::string& qname) {
            QueueMessage::ptr msgp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end()) {
                    ILOG("当前需要删除的队列不存在: %s\n", qname.c_str());
                    return;
                }    
                msgp = it->second;
                _queue_msgs.erase(qname);            
            }
            msgp->clear();
        }

        bool insert(const std::string& qname, const BasicProperties* bp, const std::string& body, const DeliveryMode& mode) {
            QueueMessage::ptr msgp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end()) {
                    ILOG("当前需要插入数据的队列不存在: %s\n", qname.c_str());
                    return false;
                }    
                msgp = it->second;          
            }
            return msgp->insert(bp, body, mode);
        }   
        
        void ack(const std::string& qname, const std::string& msg_id) {
            // 确认消息，就是将消息在队列中删除
            QueueMessage::ptr msgp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end()) {
                    ILOG("当前需要删除消息的队列不存在: %s\n", qname.c_str());
                    return;
                }  
                msgp = it->second;
            }
            msgp->remove(msg_id);
        }
        
        MessagePtr front(const std::string& qname) {
            QueueMessage::ptr msgp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end()) {
                    ILOG("当前队列不存在: %s\n", qname.c_str());
                    return MessagePtr();
                }
                msgp = it->second;
            }
            return msgp->front();
        }
        
        size_t getable_count(const std::string& qname) {
            QueueMessage::ptr msgp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end()) {
                    ILOG("当前队列不存在: %s\n", qname.c_str());
                    return 0;
                }
                msgp = it->second;                
            }
            return msgp->getable_count();
        }

        size_t total_count(const std::string& qname) {
            QueueMessage::ptr msgp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end()) {
                    ILOG("当前队列不存在: %s\n", qname.c_str());
                    return 0;
                }
                msgp = it->second;                
            }
            return msgp->total_count();
        }

        size_t waitack_count(const std::string& qname) {
            QueueMessage::ptr msgp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end()) {
                    ILOG("当前队列不存在: %s\n", qname.c_str());
                    return 0;
                }
                msgp = it->second;                
            }
            return msgp->waitack_count();            
        }

        size_t durable_count(const std::string& qname) {
            QueueMessage::ptr msgp;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _queue_msgs.find(qname);
                if (it == _queue_msgs.end()) {
                    ILOG("当前队列不存在: %s\n", qname.c_str());
                    return 0;
                }
                msgp = it->second;                
            }
            return msgp->durable_count();   
        }

    private:
        std::mutex _mutex;
        std::string _basedir;
        std::unordered_map<std::string, QueueMessage::ptr> _queue_msgs;
    };


}


#endif