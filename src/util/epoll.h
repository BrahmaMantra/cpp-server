#pragma once
#include <sys/epoll.h>

#include <unordered_map>
#include <vector>

#include "channel.h"
#include "tcpConnection.h"
// 用来管理channel的类
class Epoller {
   private:
    int epfd;
    // epoll_wait返回的事件数组，最大事件数
    struct epoll_event *events;

    mutable std::mutex mtx;  // 添加互斥锁，保证更新channel时的线程安全
    std::mutex connections_mutex;  // 保证更新all_connections时的线程安全

    // 为了方便，这里使用了一个全局的weak_ptr，指向所有的TcpConnection
    // 目的是当epoll_wait超时的时候，对epollLoop的所有连接触发心跳检测
    std::shared_ptr<std::unordered_map<int, TcpConnection *>> all_connections;

   public:
    Epoller();
    ~Epoller();

    void add_fd(int fd, uint32_t op);
    void update_channel(Channel *) const;
    void delete_channel(Channel *) const;
    std::vector<Channel *> poll(int timeout = -1);

    // 新增方法：发送Ping消息给所有连接
    void send_ping_to_all_connections() const;

    void _delete_connection(Socket *client_sock);
    void _insert_connection(int id, TcpConnection *connection);    // 插入或更新连接
    TcpConnection *_find_connection(int id);// 查找连接
};
