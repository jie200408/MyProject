#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/EventLoop.h>

class TranslateServer {
private:
    // 新连接建立成功时的回调函数
    void onConnection(const muduo::net::TcpConnectionPtr& conn) {
        if (conn->connected()) 
            std::cout << "连接建立成功!!!" << std::endl;
        else 
            std::cout << "连接建立失败!!!" << std::endl;
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

    // 通信连接收到请求时的回调函数
    void onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buff, muduo::Timestamp) {
        // 接收所有的数据
        std::string str = buff->retrieveAllAsString();
        // 接收数据接着将数据出来，然后发送数据
        std::string word = translate(str);
        conn->send(word);
    }

public:
    TranslateServer(uint16_t port)
        : _server(&_baseloop, muduo::net::InetAddress("0.0.0.0", port), 
          "TranslateServer", muduo::net::TcpServer::kReusePort)
    {
        _server.setConnectionCallback(std::bind(&TranslateServer::onConnection, this, std::placeholders::_1));
        _server.setMessageCallback(std::bind(&TranslateServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void start() {
        _server.start();
        _baseloop.loop();
    }

private:
    muduo::net::EventLoop _baseloop;
    muduo::net::TcpServer _server;
};

int main() {
    TranslateServer server(8085);
    server.start();
    return 0;
}