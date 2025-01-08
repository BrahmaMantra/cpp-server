#include "channel.h"

#include <iostream>

#include "epoll.h"
#include "eventLoop.h"
Channel::Channel(EventLoop *_loop, int _fd)
    : loop(_loop), fd(_fd), events(0), revents(0), in_epoll(false) {}

Channel::~Channel() {}

void Channel::enable_reading() {
    // EPOLLIN可读，EPOLLET边缘触发
    events = EPOLLIN | EPOLLET;
    // std::cout<<"enable_reading(): events "<<events<<" addr is
    // "<<&events<<std::endl;
    loop->update_channel(this);
}

int Channel::get_fd() { return fd; }

uint32_t Channel::get_events() { return events; }
uint32_t Channel::get_revents() { return revents; }

bool Channel::is_in_epoll() { return in_epoll; }

void Channel::set_in_epoll() { in_epoll = true; }

// void Channel::setEvents(uint32_t _ev){
//     events = _ev;
// }

void Channel::set_revents(uint32_t _ev) { revents = _ev; }

void Channel::handleEvent() { callback(); }
void Channel::set_callback(std::function<void()> _callback) {
    callback = _callback;
}