#include "channel.h"

#include <iostream>

#include "epoll.h"
#include "eventLoop.h"
Channel::Channel(EventLoop *_loop, int _fd)
    : loop(_loop),
      fd(_fd),
      events(0),
      r_events(0), 
      in_epoll(false)
{}

Channel::~Channel() {}

void Channel::enable_reading() {
    // EPOLLIN可读，EPOLLET边缘触发
    events |= EPOLLIN | EPOLLET;
    loop->update_channel(this);
}

int Channel::get_fd() { return fd; }

uint32_t Channel::get_events() { return events; }
uint32_t Channel::get_r_events() { return r_events; }

bool Channel::is_in_epoll() { return in_epoll; }

void Channel::set_in_epoll(bool _in) { in_epoll = _in; }

// void Channel::setEvents(uint32_t _ev){
//     events = _ev;
// }

void Channel::useET() {
    events |= EPOLLET;
    loop->update_channel(this);
}

void Channel::set_r_events(uint32_t _ev) { r_events = _ev; }

void Channel::handleEvent() {
    if (r_events & (EPOLLIN | EPOLLPRI)) {
        { read_callback(); }
    }
    if (r_events & EPOLLOUT) {
        writeCallback();
    }
}
void Channel::set_read_callback(std::function<void()> _callback) {
    read_callback = _callback;
}