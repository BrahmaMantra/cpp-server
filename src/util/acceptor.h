#pragma once

#include <functional>
#include <memory>

#include "util.h"
class EventLoop;
class Socket;
class InetAddress;
class Channel;
// 在client和server之间建立连接
// 本身并不负责callback的创造
class Acceptor {
   private:
    EventLoop *loop;  // 后续考虑用弱引用实现
    Socket *acceptor_sock;
    std::unique_ptr<InetAddress> addr;
    std::unique_ptr<Channel> acceptor_channel;

    // 目前是由server的构造函数来决定的，后续应该根据客户端的需求从表里面选择
    std::function<void(Socket *accept_sock)> acceptor_callback;

   public:
    DISALLOW_COPY_AND_MOVE(Acceptor);

    Acceptor(EventLoop *_loop, std::unique_ptr<InetAddress> _addr);
    ~Acceptor();
    void accept_connection();
    void set_new_connection_callback(std::function<void(Socket *)> cb);
};