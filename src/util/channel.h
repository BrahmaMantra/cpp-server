#pragma once
#include <sys/epoll.h>

#include <functional>
#include <iostream>

#include "eventLoop.h"
#include "util.h"
class Epoller;
//封装Epoll_events
class Channel {
   private:
    EventLoop *loop;
    int fd;
    // 注册事件时设置的,用户感兴趣的事件类型
    uint32_t listen_events;
    // 内核返回的,实际发生的事件类型
    uint32_t ready_events;
    bool in_epoll;
    // 在EventLoop里执行的回调函数
    std::function<void()> read_callback;
    std::function<void()> write_callback;
    std::function<void()> close_callback;

   public:
    DISALLOW_COPY_AND_MOVE(Channel);
    Channel(EventLoop *_loop, int _fd);
    ~Channel();

    void enable_read();
    void enable_write();

    int get_fd();
    uint32_t get_listen_events();
    uint32_t get_ready_events();
    bool is_in_epoll();
    void set_in_epoll(bool _in = true);
    void enable_ET();

    // void setEvents(uint32_t);
    void set_ready_events(uint32_t);

    void handleEvent();
    void set_read_callback(std::function<void()> const &callback);
    void set_write_callback(std::function<void()> const &callback);
    void set_close_callback(std::function<void()> const &callback);
};
