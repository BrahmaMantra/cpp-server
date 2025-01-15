#pragma once
#include "threadPool/threadPool.h"
#include "util.h"
#include <memory>
#include <unordered_map>
#include"tcpConnection.h"
class Epoller;
class Channel;
class Socket;
class TcpConnection; // 前向声明 TcpConnection
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
    void update_channel(Channel *) const;
    void delete_channel(Channel *ch) const;

    void delete_connection(Socket *client_sock);
    void insert_connection(int id, TcpConnection *connection);    // 插入或更新连接
    TcpConnection *find_connection(int id);// 查找连接
};