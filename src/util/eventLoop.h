#pragma once
#include <memory>
#include <unordered_map>

#include "tcpConnection.h"
#include "threadPool/threadPool.h"
#include "util.h"
class Epoller;
class Channel;
class Socket;
class TcpConnection;  // 前向声明 TcpConnection
class EventLoop {
   private:
    std::unique_ptr<Epoller> ep;
    bool quit;  // 退出标志，目前没有用到
    int loop_id;

   public:
    DISALLOW_COPY_AND_MOVE(EventLoop);
    EventLoop(int loop_id);
    ~EventLoop();

    void loop() const;
    // 会同时更新connections
    void update_channel(std::shared_ptr<Channel> ch);

    // 删除connection就一定会修改channel
    void delete_connection(Socket* client_sock);

    void insert_connection(
        int id, std::shared_ptr<TcpConnection> connection);  // 插入或更新连接
    std::shared_ptr<TcpConnection> find_connection(int id);  // 查找连接
};