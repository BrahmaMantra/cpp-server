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
    acceptor_sock = new Socket();
    addr = new InetAddress("127.0.0.1", 7777);
    acceptor_sock->bind(addr);
    acceptor_sock->listen();
    acceptor_sock->setNonblocking();
    acceptor_channel = new Channel(loop, acceptor_sock->get_fd());

    // 为了在Epoll中监听accept事件，需要设置一个回调函数
    // 如果在某个时刻你改变了acceptor_callback，那么channel_callback在执行时
    // 会调用最新的acceptor_callback。这是因为channel_callback捕获的是this指针
    // 而不是acceptor_callback的值。
    std::function<void()> channel_callback = [this]() {
        accept_connection();
    };
    // 将当前Channel的回调函数设置为channel_callback
    acceptor_channel->set_read_callback(channel_callback);
    acceptor_channel->enable_reading();
    acceptor_channel->set_use_threadPool(false);
    delete addr;
}

/**
 * @brief 析构函数，清理Acceptor对象
 */
Acceptor::~Acceptor() {
    delete acceptor_sock;
    delete addr;
    delete acceptor_channel;
}

/**
 * @brief 接受新连接并调用回调函数
 */
// houxvzaizheli fenpei 
void Acceptor::accept_connection() {
     InetAddress *client_addr = new InetAddress();
     Socket * client_sock = new Socket(acceptor_sock->accept(client_addr));
     client_sock->setNonblocking();
    // new_connection()
     acceptor_callback(client_sock);
    }

/**
 * @brief 设置新的连接回调函数
 *
 * @param cb 接受一个Socket指针参数的回调函数
 */
void Acceptor::set_new_connection_callback(std::function<void(Socket *)> cb) {
    acceptor_callback = cb;
}