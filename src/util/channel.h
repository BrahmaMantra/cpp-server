#pragma once
#include <sys/epoll.h>

#include <functional>

#include "eventLoop.h"
class Epoll;
//封装Epoll_events
class Channel {
   private:
    EventLoop *loop;
    int fd;
    //表示希望监听这个文件描述符的哪些事件（感兴趣的）
    uint32_t events;
    //在epoll返回该Channel时文件描述符正在发生的事件（实际发生）
    uint32_t revents;
    bool in_epoll;
    bool use_threadPool;
    // 执行的回调函数
    std::function<void()> read_callback;
    std::function<void()> writeCallback;

   public:
    Channel(EventLoop *_loop, int _fd);
    ~Channel();

    void enable_reading();

    int get_fd();
    uint32_t get_events();
    uint32_t get_revents();
    bool is_in_epoll();
    void set_in_epoll(bool _in = true);
    void useET();

    // void setEvents(uint32_t);
    void set_revents(uint32_t);

    void handleEvent();
    void set_read_callback(std::function<void()>);
    void set_use_threadPool(bool use = true);
};
