#ifndef __M_CONNECTION_H__
#define __M_CONNECTION_H__
#include "channel.hpp"

namespace mq {
    class Connection {
    private:
        void basicResponce(bool ok, const std::string& rid, const std::string& cid) {
            basicCommonResponce resp;
            resp.set_cid(cid);
            resp.set_rid(rid);
            resp.set_ok(ok);
            _codec->send(_conn, resp);
        }

    public:
    using ptr = std::shared_ptr<Connection>;

    Connection(const VirtualHost::ptr& host, 
        const ConsumerManager::ptr& cmp, 
        const ProtobufCodecPtr& codec, 
        const muduo::net::TcpConnectionPtr conn, 
        const threadpool::ptr& pool) 
        : _conn(conn),
          _codec(codec),
          _cmp(cmp),
          _host(host),
          _pool(pool),
          _channels(std::make_shared<ChannelManager>())
    {}

    void openChannel(const openChannelRequestPtr& req) {
        // 先检查是否存在
        bool ret = _channels->openChannel(req->cid(), _host, _cmp, _codec, _conn, _pool);
        if (ret == false) {
            DLOG("信道已经存在，信道ID重复\n");
            return this->basicResponce(false, req->rid(), req->cid());
        }
        ILOG("%s 信道创建成功\n", req->cid().c_str());
        this->basicResponce(true, req->rid(), req->cid());
    }
    
    void closeChannel(const closeChannelRequestPtr& req) {
        _channels->closeChannel(req->cid());
        this->basicResponce(true, req->rid(), req->cid());
    }

    Channel::ptr getChannel(const std::string& cid) {
        return _channels->getChannel(cid);
    }

    ~Connection() {}

    private:
        // 一个连接模块处理多个信道，一个信道处理一个消费者
        muduo::net::TcpConnectionPtr _conn;        // 信道关联的连接
        ProtobufCodecPtr _codec;                   //协议处理器，protobuf协议处理句柄  
        ConsumerManager::ptr _cmp;                 // 消费者管理句柄
        VirtualHost::ptr _host;                    // 虚拟机
        threadpool::ptr _pool;                     // 线程池    
        ChannelManager::ptr _channels;             // 管理多个信道的句柄
    };

    class ConnectionManager {
    public:
        using ptr = std::shared_ptr<ConnectionManager>;

        ConnectionManager() {}

        void newConnection(const VirtualHost::ptr& host, 
            const ConsumerManager::ptr& cmp, 
            const ProtobufCodecPtr& codec, 
            const muduo::net::TcpConnectionPtr conn, 
            const threadpool::ptr& pool) {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _conns.find(conn);
            if (it != _conns.end())
                return;
            Connection::ptr connection = std::make_shared<Connection>(host, cmp, codec, conn, pool);
            _conns[conn] = connection;
        }

        void delConnection(const muduo::net::TcpConnectionPtr conn) {
            std::unique_lock<std::mutex> lock(_mutex);
            _conns.erase(conn);
        }

        Connection::ptr getConnection(const muduo::net::TcpConnectionPtr conn) {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _conns.find(conn);
            if (it == _conns.end())
                return Connection::ptr();
            return it->second;
        }
    private:
        // 用于管理多个连接
        std::mutex _mutex;
        std::unordered_map<muduo::net::TcpConnectionPtr, Connection::ptr> _conns;
    };
}

#endif