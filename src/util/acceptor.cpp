#include "acceptor.h"

#include "InetAddress.h"
#include "channel.h"
#include "server.h"
#include "socket.h"

/**
 * @brief 构造函数，初始化Acceptor对象
 *
 * @param _loop 指向EventLoop对象的指针
 */
Acceptor::Acceptor(EventLoop *_loop) : loop(_loop) {
    sock = new Socket();
    addr = new InetAddress("127.0.0.1", 7777);
    sock->bind(addr);
    sock->listen();
    sock->setNonblocking();
    accept_channel = new Channel(loop, sock->get_fd());
    // server的socket在这里是不可变的，所以这里的callback是固定的
    std::function<void()> callback =
        std::bind(&Acceptor::accept_connection, this);
    accept_channel->set_callback(callback);
    accept_channel->enable_reading();
}

/**
 * @brief 析构函数，清理Acceptor对象
 */
Acceptor::~Acceptor() {
    delete sock;
    delete addr;
    delete accept_channel;
}

/**
 * @brief 接受新连接并调用回调函数
 */
void Acceptor::accept_connection() { acceptor_callback(sock); }

/**
 * @brief 设置新的连接回调函数
 *
 * @param cb 接受一个Socket指针参数的回调函数
 */
void Acceptor::set_new_connection_callback(std::function<void(Socket *)> cb) {
    acceptor_callback = cb;
}