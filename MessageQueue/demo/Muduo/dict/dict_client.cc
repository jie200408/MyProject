#include <iostream>
#include <string>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/EventLoopThread.h>

class TranslateClient {
private:
    // 连接建立成功时候的回调函数，连接建立成功后，唤醒上边的阻塞
    void onConnection(const muduo::net::TcpConnectionPtr& conn) {
        if (conn->connected()) {
            // 唤醒主线程中的线程
            _latch.countDown();
            _conn = conn;
        } else {
            _conn.reset();
        }
    }

    // 收到消息时候的回调函数
    void onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buff, muduo::Timestamp) {
        std::cout << "answer: " << buff->retrieveAllAsString() << std::endl;
    }


public:
    TranslateClient(const std::string& ip, uint16_t port)
        : _latch(1),
          _client(_loopthread.startLoop(), muduo::net::InetAddress(ip, port), "TranslateClient")
    {
        _client.setConnectionCallback(std::bind(&TranslateClient::onConnection, this, std::placeholders::_1));
        _client.setMessageCallback(std::bind(&TranslateClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    }
    // 阻塞等待连接建立成功之后再返回
    void connect() {
        _client.connect();
        // 阻塞等待，知道连接成功
        _latch.wait();
    }

    bool send(const std::string& msg) {
        if (_conn->connected()) {
            _conn->send(msg);
            return true;
        }
        return false;
    }
private:
    muduo::CountDownLatch _latch;
    muduo::net::EventLoopThread _loopthread;
    muduo::net::TcpClient _client;
    muduo::net::TcpConnectionPtr _conn;
};

int main() {
    TranslateClient client("127.0.0.1", 8085);
    client.connect();
    
    while (true) {
        std::string buf;
        std::cin >> buf;
        client.send(buf);
    }

    return 0;
}