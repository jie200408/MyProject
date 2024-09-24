#include "../../include/proto/codec.h"
#include "../../include/proto/dispatcher.h"
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/TcpServer.h>
#include <iostream>
#include <unordered_map>
#include "request.pb.h"
#include <functional>

class Server {
private:
    using MessagePtr = std::shared_ptr<google::protobuf::Message>;
    using TranslateRequestPtr = std::shared_ptr<protocal::TranslateRequest>;
    using TranslateResponcePtr = std::shared_ptr<protocal::TranslateResponce>;
    using AddRequestPtr = std::shared_ptr<protocal::AddRequest>;
    using AddResponcePtr = std::shared_ptr<protocal::AddResponce>;


    void onConnection(const muduo::net::TcpConnectionPtr& conn) {
        if (conn->connected()) 
            LOG_INFO << "新连接建立成功!";
        else 
            LOG_INFO << "连接关闭!";            
    }

    // 默认的处理函数
    void unUnKnownMessage(const muduo::net::TcpConnectionPtr& conn, const MessagePtr& message, muduo::Timestamp) {
        LOG_INFO << "unUnKnownMessage" << message->GetTypeName();
        conn->shutdown(); // 关闭连接
    }

    void onAdd(const muduo::net::TcpConnectionPtr& conn, const AddRequestPtr& message, muduo::Timestamp) {
        int num1 = message->num1();
        int num2 = message->num2();
        int result = num1 + num2;
        protocal::AddResponce resp;
        resp.set_result(result);
        _codec.send(conn, resp);
    }

    void onTranslate(const muduo::net::TcpConnectionPtr& conn, const TranslateRequestPtr& message, muduo::Timestamp) {
        std::string req_msg = message->msg();
        std::string rsp_msg = translate(req_msg);
        // 组织protobuf的响应
        protocal::TranslateResponce resp;
        resp.set_msg(rsp_msg);
        _codec.send(conn, resp);
    }

    std::string translate(const std::string& word) {
        static std::unordered_map<std::string, std::string> dict = {
            {"hello", "你好"},
            {"retrieve", "取回"},
            {"map", "地图"},
            {"pen", "笔"}
        };
        auto it = dict.find(word);
        if (it == dict.end()) 
            return "none";
        else
            return it->second;
    }

public:
    Server(uint16_t port)
        : _server(&_baseloop, muduo::net::InetAddress("0.0.0.0", port), "Server", muduo::net::TcpServer::kReusePort),
          _dispathcher(std::bind(&Server::unUnKnownMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
          _codec(std::bind(&ProtobufDispatcher::onProtobufMessage, &_dispathcher, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
    {
        // 现在注册业务处理请求函数
        _dispathcher.registerMessageCallback<protocal::TranslateRequest>(std::bind(&Server::onTranslate, this,
             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        _dispathcher.registerMessageCallback<protocal::AddRequest>(std::bind(&Server::onAdd, this,
             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        _server.setMessageCallback(std::bind(&ProtobufCodec::onMessage, &_codec, 
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        _server.setConnectionCallback(std::bind(&Server::onConnection, this, std::placeholders::_1));
    }      

    void start() {
        _server.start();
        _baseloop.loop();
    }

private:
    muduo::net::EventLoop _baseloop;
    muduo::net::TcpServer _server;   // 服务器对象
    ProtobufDispatcher _dispathcher; // 请求分发器对象，需要向分发器中的注册处理函数
    ProtobufCodec _codec;            // protobuf 协议处理器，针对收到的请求数据进行protobuf协议处理 
};

int main() {
    Server server(8085);
    server.start();
    return 0;
}