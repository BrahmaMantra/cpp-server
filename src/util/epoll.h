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

    std::mutex channels_mtx;  // 保护channels的互斥锁
    std::mutex connections_mutex;  // 保证更新connections时的线程安全

    // 目的是当epoll_wait超时的时候，对epollLoop的所有连接触发心跳检测
    std::unordered_map<int, std::shared_ptr<TcpConnection>>connections;
    std::unordered_map<int, std::shared_ptr<Channel>> channels;

   public:
    Epoller();
    ~Epoller();

    void update_channel(std::weak_ptr<Channel> ch) ;
    void delete_channel(std::shared_ptr<Channel>);
    std::vector<Channel *> poll(int timeout = -1);

    // 新增方法：发送HEARTBEAT_MSG消息给所有连接
    std::vector<int> send_HEARTBEAT_to_all_connections();

    void _delete_connection(Socket *client_sock);
    void _insert_connection(int id, std::shared_ptr<TcpConnection> connection);    // 插入或更新连接
    std::shared_ptr<TcpConnection> _find_connection(int id);// 查找连接

    std::unordered_map<int, std::shared_ptr<Channel>>& get_channels()  {
         return channels; 
        }
    std::unordered_map<int, std::shared_ptr<TcpConnection>>& get_connections() {
        return connections;
    }

    void _delete_fd_in_channels(int fd);
};
