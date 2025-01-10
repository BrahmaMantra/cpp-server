#pragma once

#include <functional>

class EventLoop;
class Socket;
class InetAddress;
class Channel;
// 在client和server之间建立连接
// 本身并不负责callback的创造
class Acceptor {
   private:
    EventLoop *loop;
    Socket *acceptor_sock;
    InetAddress *addr;
    Channel *acceptor_channel;

   public:
    Acceptor(EventLoop *_loop);
    ~Acceptor();
    void accept_connection();

    // 目前是由server的构造函数来决定的
    std::function<void(Socket *accept_sock)> acceptor_callback;
    void set_new_connection_callback(std::function<void(Socket *)> cb);
};