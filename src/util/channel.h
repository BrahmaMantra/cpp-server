#pragma once
#include <sys/epoll.h>
#include<iostream>
#include <functional>

#include "eventLoop.h"
class Epoll;
//封装Epoll_events
class Channel {
   private:
    EventLoop *loop;
    int fd;
    // 注册事件时设置的,用户感兴趣的事件类型
    uint32_t events;
    // 内核返回的,实际发生的事件类型
    uint32_t r_events;
    bool in_epoll;
    // 在EventLoop里执行的回调函数
    std::function<void()> read_callback;
    std::function<void()> writeCallback;
   public:
    Channel(EventLoop *_loop, int _fd);
    ~Channel();

    void enable_reading();

    int get_fd();
    uint32_t get_events();
    uint32_t get_r_events();
    bool is_in_epoll();
    void set_in_epoll(bool _in = true);
    void useET();

    // void setEvents(uint32_t);
    void set_r_events(uint32_t);

    void handleEvent();
    void set_read_callback(std::function<void()>);
};
