#include "../../include/proto/codec.h"
#include "../../include/proto/dispatcher.h"
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/base/CountDownLatch.h>
#include <iostream>
#include "request.pb.h"

// class Client {
// private:
//     using AddResponcePtr = std::shared_ptr<protocal::AddResponce>;
//     using TranslateResponcePtr = std::shared_ptr<protocal::TranslateResponce>;
//     using MessagePtr = std::shared_ptr<google::protobuf::Message>;

//     bool send(const google::protobuf::Message* message) {
//         // 连接状态正常才会发送
//         if (_conn->connected()) {
//             _codec.send(_conn, *message);
//             return true;
//         }
//         return false;
//     }

//     void onTranslate(const muduo::net::TcpConnectionPtr& conn, TranslateResponcePtr& message, muduo::Timestamp) {
//         std::cout << "翻译结果：" << message->msg() << std::endl;
//     }

//     void onAdd(const muduo::net::TcpConnectionPtr& conn, AddResponcePtr& message, muduo::Timestamp) {
//         std::cout << "加法结果：" << message->result() << std::endl;            
//     }

//     // 默认的处理函数
//     void unUnKnownMessage(const muduo::net::TcpConnectionPtr& conn, const MessagePtr& message, muduo::Timestamp) {
//         LOG_INFO << "unUnKnownMessage" << message->GetTypeName();
//         conn->shutdown(); // 关闭连接
//     }

//     void onConnection(const muduo::net::TcpConnectionPtr& conn) {
//         if (conn->connected()) {
//             // 连接成功，唤醒主线程中的阻塞
//             _latch.countDown();
//             _conn = conn;
//         } else {
//             // 连接关闭重置
//             _conn.reset();
//         }
//     }

// public:
//     Client(const std::string& ip, uint16_t port)
//         : _latch(1),
//           _client(_loopthread.startLoop(), muduo::net::InetAddress(ip, port), "Client"),
//           _dispatcher(std::bind(&Client::unUnKnownMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
//           _codec(std::bind(&ProtobufDispatcher::onProtobufMessage, &_dispatcher, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))         
//     {
//         // _dispatcher.registerMessageCallback<protocal::TranslateResponce>(std::bind(&Client::onTranslate, this, 
//         //     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

//         // _dispatcher.registerMessageCallback<protocal::TranslateResponce>(std::bind(&Client::onTranslate, this, 
//         //         std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
//         // _dispatcher.registerMessageCallback<protocal::AddResponce>(std::bind(&Client::onAdd, this,
//         //         std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

//         _dispatcher.registerMessageCallback<protocal::TranslateResponce>(std::bind(&Client::onTranslate, this,
//              std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
//         _dispatcher.registerMessageCallback<protocal::AddResponce>(std::bind(&Client::onAdd, this,
//              std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));        

//         _client.setMessageCallback(std::bind(&ProtobufCodec::onMessage, &_codec, 
//                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

//          _client.setConnectionCallback(std::bind(&Client::onConnection, this, std::placeholders::_1));      
//     }

//     void connect() {
//         _client.connect();
//         _latch.wait();
//     }

//     void Translate(const std::string& msg) {
//         protocal::TranslateRequest req;
//         req.set_msg(msg);
//         send(&req);
//     }

//     void Add(int num1, int num2) {
//         protocal::AddRequest req;
//         req.set_num1(num1);
//         req.set_num2(num2);
//         send(&req);
//     }

// private:
//     muduo::CountDownLatch _latch;            // 同步器
//     muduo::net::EventLoopThread _loopthread; // 异步循环处理线程
//     muduo::net::TcpConnectionPtr _conn;      // 客户端的连接
//     muduo::net::TcpClient _client;           // 客户端
//     ProtobufDispatcher _dispatcher;           // 请求分发器
//     ProtobufCodec _codec;                    // 协议处理器
// };



class Client {
private:
    using AddResponcePtr = std::shared_ptr<protocal::AddResponce>;
    using TranslateResponcePtr = std::shared_ptr<protocal::TranslateResponce>;
    using MessagePtr = std::shared_ptr<google::protobuf::Message>;
    bool send(const google::protobuf::Message *message) {
        if (_conn->connected()) {//连接状态正常，再发送，否则就返回false
            _codec.send(_conn, *message);
            return true;
        }
        return false;
    }  
    void onTranslate(const muduo::net::TcpConnectionPtr& conn, const TranslateResponcePtr& message, muduo::Timestamp) {
        std::cout << "翻译结果：" << message->msg() << std::endl;
    }
    void onAdd(const muduo::net::TcpConnectionPtr& conn, const AddResponcePtr& message, muduo::Timestamp) {
        std::cout << "加法结果：" << message->result() << std::endl;
    }
    void onUnknownMessage(const muduo::net::TcpConnectionPtr& conn, const MessagePtr& message, muduo::Timestamp) {
        LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
        conn->shutdown();
    }
    void onConnection(const muduo::net::TcpConnectionPtr&conn){
        if (conn->connected()) {
            _latch.countDown();//唤醒主线程中的阻塞
            _conn = conn;
        }else {
            //连接关闭时的操作
            _conn.reset();
        }
    }    
public:
        // typedef std::shared_ptr<google::protobuf::Message> MessagePtr;
        // typedef std::shared_ptr<protocal::AddResponce> AddResponsePtr;
        // typedef std::shared_ptr<protocal::TranslateResponce> TranslateResponsePtr;
    Client(const std::string &sip, int sport):
        _latch(1), _client(_loopthread.startLoop(), muduo::net::InetAddress(sip, sport), "Client"),
        _dispatcher(std::bind(&Client::onUnknownMessage, this, std::placeholders::_1, 
            std::placeholders::_2, std::placeholders::_3)),
        _codec(std::bind(&ProtobufDispatcher::onProtobufMessage, &_dispatcher, 
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)){

        _dispatcher.registerMessageCallback<protocal::TranslateResponce>(std::bind(&Client::onTranslate, this, 
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                
        _dispatcher.registerMessageCallback<protocal::AddResponce>(std::bind(&Client::onAdd, this, 
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        _client.setMessageCallback(std::bind(&ProtobufCodec::onMessage, &_codec,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        _client.setConnectionCallback(std::bind(&Client::onConnection, this, std::placeholders::_1));      
    }
    void connect() {
        _client.connect();
        _latch.wait();//阻塞等待，直到连接建立成功
    }
    void Translate(const std::string &msg){
        protocal::TranslateRequest req;
        req.set_msg(msg);
        send(&req);
    }
    void Add(int num1, int num2) {
        protocal::AddRequest req;
        req.set_num1(num1);
        req.set_num2(num2);
        send(&req);
    }

private:
    muduo::CountDownLatch _latch;//实现同步的
    muduo::net::EventLoopThread _loopthread;//异步循环处理线程
    muduo::net::TcpConnectionPtr _conn;//客户端对应的连接
    muduo::net::TcpClient _client;//客户端
    ProtobufDispatcher _dispatcher;//请求分发器
    ProtobufCodec _codec;//协议处理器    
};




int main() {
    Client client("127.0.0.1", 8085);
    client.connect();
    std::cout << "xxx" << std::endl;
    client.Translate("hello");
    client.Add(11, 22);

    sleep(1);
    return 0;    
}