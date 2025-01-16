#include "channel.h"

#include <iostream>
#include<mutex>
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

// 不要enable_read()完就立刻调用update_channel()，因为可能这样会导致
// 在enable_ET()之前，已经加入epoll_wait()中的fd被删除，也就是资源竞争
void Channel::enable_read() {
    // EPOLLIN可读，EPOLLET边缘触发,EPOLLRDHUP对端关闭连接
    listen_events |= EPOLLIN | EPOLLET | EPOLLRDHUP;
}
// TODO!
void Channel::enable_write() {
    listen_events |= EPOLLOUT | EPOLLRDHUP;
}
void Channel::enable_ET() {
    listen_events |= EPOLLET;
}

void Channel::set_ready_events(uint32_t _ev) { ready_events = _ev; }

void Channel::handleEvent() {
    if (ready_events & (EPOLLHUP | EPOLLRDHUP)) {
        // 对端关闭连接
        if (close_callback) {
            DEBUG_PRINT("handleEvent(): fd %d closed\n", fd);
            close_callback();
        } else {
            std::cerr << "handleEvent(): fd " << fd << " has NULL close_callback()" << std::endl;
        }
        return;
    } else if (ready_events & (EPOLLIN | EPOLLPRI)) {
        // 可读事件 | 优先级事件
        if (read_callback) {
            read_callback();
        } else {
            std::cerr << "handleEvent(): fd " << fd << " has NULL read_callback()" << std::endl;
        }
    } else if (ready_events & EPOLLOUT) {
        // 可写事件
        if (write_callback) {
            write_callback();
        } else {
            std::cerr << "handleEvent(): fd " << fd << " has NULL write_callback()" << std::endl;
        }
    } else {
        std::cerr << "handleEvent(): fd " << fd << " has unexcepted event" << std::endl;
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