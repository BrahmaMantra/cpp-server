#include "channel.h"

#include <iostream>

#include "epoll.h"
#include "eventLoop.h"
Channel::Channel(EventLoop *_loop, int _fd)
    : loop(_loop),
      fd(_fd),
      listen_events(0),
      ready_events(0),
      in_epoll(false) {}

Channel::~Channel() {}

int Channel::get_fd() { return fd; }

uint32_t Channel::get_listen_events() { return listen_events; }
uint32_t Channel::get_ready_events() { return ready_events; }

bool Channel::is_in_epoll() { return in_epoll; }

void Channel::set_in_epoll(bool _in) { in_epoll = _in; }

void Channel::enable_read() {
    // EPOLLIN可读，EPOLLET边缘触发,EPOLLRDHUP对端关闭连接
    listen_events |= EPOLLIN | EPOLLET | EPOLLRDHUP;
    loop->update_channel(this);
}
// TODO!
void Channel::enable_write() {
    listen_events |= EPOLLOUT | EPOLLRDHUP;
    loop->update_channel(this);
}
void Channel::enable_ET() {
    listen_events |= EPOLLET;
    loop->update_channel(this);
}

void Channel::set_ready_events(uint32_t _ev) { ready_events = _ev; }

void Channel::handleEvent() {
    // 对端关闭连接
    if (ready_events & (EPOLLHUP | EPOLLRDHUP)) {
        DEBUG_PRINT("handleEvent(): fd %d closed\n", fd);
        close_callback();
        return ;
    }
    // 可读事件 | 优先级事件
    if (ready_events & (EPOLLIN | EPOLLPRI)) {
        read_callback();
    }
    // 可写事件
    if (ready_events & EPOLLOUT) {
        write_callback();
    }
}
void Channel::set_read_callback(std::function<void()> const &_callback) {
    read_callback = _callback;
}
// 暂时没用到，但是应该要有
void Channel::set_write_callback(std::function<void()> const &callback) {
    write_callback = callback;
}
void Channel::set_close_callback(std::function<void()> const &callback) {
    close_callback = callback;
}