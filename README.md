# Message Queue

[TOC]

## 简介

​	实现了一个基于AMQP（Advanced Message Queuing Protocol）协议的消息订阅与转发系统，主要实现的是将通过三种不同的订阅模式，将消息路由转发到对应的用户手中。

> 三种订阅模式为：
>
> - 主题订阅：当订阅了某一类信息时，那么将会接收到这一类的信息，比如我订阅的是运动新闻，那么将会收到的来自篮球、足球、 排球等各类新闻，但是其他的娱乐新闻却收不到。
> - 直连订阅：当订阅了某一种信息时，只能收到这种信息，比如我指定订阅了NBA的新闻，那么只会接收到来自NBA的新闻。
> - 广播订阅：当订阅了某一个消息发布者之后，将会收到来自该发布者发布的所有信息。

​	整个系统分为三端：服务器端、消息发送端、消息接收端。消息接收端和消息发送端都可以声明不同订阅模式的交换机以及消息队列，同时将交换机与消息队列通过特定的绑定关系绑定起来。

​	当消息由消息发送端发送至服务器时，首先是发送至服务器内的交换机中，接着通过消息自带的路由标签和交换机与队列之间的绑定关系进行路由匹配，然后发送给对应的消息队列，最后通过消息队列发送到对应的消息接收端。

## 安装环境

​	接下来便是进行环境配置。该代码是在Linux下开发的，调用了一些系统调用接口，所以只能在Linux环境下运行。一下环境是本人在ubuntu22.04版本下配置的环境。

### 基础环境搭建

​	先更新软件包源

```
sudo apt update
```

​	安装wget

```
sudo apt-get install wget
```

​	安装编译器

```
sudo apt-get install gcc g++
```

​	按章项目构建工具

```
 sudo apt-get install make
```

​	安装git

```
sudo apt-get install git
```

​	安装cmake

```
sudo apt-get install cmake
```

### 安装protobuf

​	移除现有的protobuf（因为不同版本的protobuf可能存在编译不过的情况，本项目使用的是protobuf 3.14.0 版本）

```
sudo apt-get remove --purge protobuf-compiler libprotobuf-dev libprotobuf17
```

​	安装编译源码所需的工具以及依赖

```
sudo apt-get install -y build-essential autoconf automake libtool curl make g++ unzip
```

​	下载protobuf 3.14.0的源码包

```
curl -LO https://github.com/protocolbuffers/protobuf/releases/download/v3.14.0/protobuf-all-3.14.0.tar.gz
```

​	解压并进入目录

```
tar -xzvf protobuf-all-3.14.0.tar.gz
cd protobuf-3.14.0
```

​	编译安装

```
./autogen.sh

./configure

make

sudo make install

// 检查版本
protoc --version
```

​	将库加入到动态连接库的配置文件（读者也可选择通过其他的方式来实现库链接，比如环境变量）

```
sudo nano /etc/ld.so.conf.d/local.conf

// 打开之后在文件中写入
/usr/local/lib
```

​	更新动态连接库缓存

```
sudo idconfig
```

### 安装sqlite3

​	本项目安装的sqlite版本为sqlite3 3.37.2版本。

​	先获取对应版本

```
wget https://www.sqlite.org/2022/sqlite-autoconf-3370200.tar.gz
```

​	解压源码

```
tar -xvf sqlite-autoconf-3370200.tar.gz
cd sqlite-autoconf-3370200
```

​	编译和安装

```
./configure --prefix=/usr/local

make

sudo make install
```

​	查看版本

```
sqlite3 --version
3.37.2 2022-01-06 13:25:41 872ba256cbf61d9290b571c0e6d82a20c224ca3ad82971edc46b29818d5dalt1
```

### 安装muduo库

​	下载源码

```
git clone https://github.com/chenshuo/muduo.git
```

​	安装对应的依赖环境

```
sudo apt-get install libz-dev libboost-all-dev
```

​	运行脚本编译安装

```
unzip muduo-master.zip

cd muduo-master

./build.sh

./build.sh install
```

​	切换编译目录

```
// 就在上级目录中的build目录下
cd ../build/release-cpp11/bin

// 运行程序，查看是否安装成功
// 服务端
./protobuf_server 9091
// 客户端
./protobuf_client 0.0.0.0 9091
// 若成功运行则说明安装成功
```

​	头文件以及库文件的链接（muduo下载之后的头文件以及库文件不会自动加载到对应的目录或者环境变量中，需要复制到对应的目录或者加入环境变量才能编译成功）

​	先把库文件复制到/usr/local/lib/目录下

```
// 先找到对应文件目录
sudo find / -name "libmuduo_net*"
// /home/jzhong/code/bag/build/release-cpp11/lib/libmuduo_net.a (说明在/home/jzhong/code/bag/build/release-cpp11/lib/)目录下

// 将该目录下的所有文件复制到/usr/local/lib/
sudo cp 刚刚找到的目录/lib/*.a /usr/local/lib/

// 更新链接器缓存
sudo ldconfig
```

​	然后把头文件复制到/usr/local/include/目录下

```
// 先找到对应的头文件目录
sudo find / -name "Logging.h"
// /home/jzhong/code/bag/muduo-master/muduo/base/Logging.h(将Logging.h上两级目录整个muduo目录都移动到/usr/local/include/目录下)

sudo cp -r /home/jzhong/code/bag/muduo-master/muduo /usr/local/include/
```

​	以上这两种方法只是我提供的一种方法，读者也可以选择其他的方法。

## 编译

​	当前一共存在两端：客户端和服务前端，分别编译执行这两个即可，安装好以上环境之后，分别在对应路径下进行编译即可，如下：

​	编译客户端：

![client](.\image\client.gif)

​	编译服务器端：

![server](.\image\server.gif)

## 运行

​	运行前，需要先运行服务前端，让其可以接收对应的连接以及请求。如下：

![1](.\image\1.png)

​	当接收到对应的连接时会打印相关的日志。

​	以下的测试运行将从五个方面进行：分别是用户管理、消息的发送和接收、用户数据管理、系统数据管理、订阅管理

### 用户管理

​	用户的登陆与注册：

![image-20241224232027606](.\image\image-20241224232027606.png)

​	用户修改个人信息：

![image-20241224232242443](.\image\image-20241224232242443.png)

​	用户列表显示：

![image-20241224232429831](.\image\image-20241224232429831.png)

​	修改/删除用户：

![image-20241224232544644](.\image\image-20241224232544644.png)

### 消息的发送和接收

​	消息接收发布前的准备工作：

![image-20241224232705876](.\image\image-20241224232705876.png)

![image-20241224232711477](.\image\image-20241224232711477.png)

​	消息发布用户编辑要发送的信息：

![image-20241224232849081](.\image\image-20241224232849081.png)

​	发送消息和接收消息：

![image-20241224232924486](.\image\image-20241224232924486.png)

​	查看信箱数据：

![image-20241224232919773](.\image\image-20241224232919773.png)

### 用户数据管理

​	垃圾回收(回收服务前端中无用的信息)：

![image-20241224233313372](.\image\image-20241224233313372.png)

​	管理员查看/修改用户数据：

![image-20241224233331337](.\image\image-20241224233331337.png)

### 系统数据管理

​	管理员查看/修改交换机数据

![image-20241224233514017](.\image\image-20241224233514017.png)

​	管理员查看/修改队列数据

![image-20241224233522934](.\image\image-20241224233522934.png)

​	管理员查看/修改绑定关系数据

![image-20241224233540443](.\image\image-20241224233540443.png)

### 订阅管理

​	管理员查看用户的订阅

![image-20241224233846104](.\image\image-20241224233846104.png)

​	管理员删除用户的订阅

![image-20241224233850735](.\image\image-20241224233850735.png)

​	管理员修改用户的订阅

![image-20241224233855070](.\image\image-20241224233855070.png)