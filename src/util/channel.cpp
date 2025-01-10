#include "channel.h"

#include <iostream>

#include "epoll.h"
#include "eventLoop.h"
Channel::Channel(EventLoop *_loop, int _fd)
    : loop(_loop), fd(_fd), events(0), revents(0), in_epoll(false),use_threadPool(true) {}

Channel::~Channel() {}

void Channel::enable_reading() {
    // EPOLLIN可读，EPOLLET边缘触发
    events |= EPOLLIN | EPOLLET;
    loop->update_channel(this);
}

int Channel::get_fd() { return fd; }

uint32_t Channel::get_events() { return events; }
uint32_t Channel::get_revents() { return revents; }

bool Channel::is_in_epoll() { return in_epoll; }

void Channel::set_in_epoll(bool _in) { in_epoll = _in; }

// void Channel::setEvents(uint32_t _ev){
//     events = _ev;
// }

void Channel::useET() {
    events |= EPOLLET;
    loop->update_channel(this);
}

void Channel::set_revents(uint32_t _ev) { revents = _ev; }

void Channel::handleEvent() { 
        if(revents & (EPOLLIN | EPOLLPRI)){
            if(use_threadPool){
                loop->add_task(read_callback);
            }else{
                read_callback();
            }
        }
        if(revents & EPOLLOUT){
            if(use_threadPool){
                loop->add_task(writeCallback);
            }else{
                writeCallback();
            }
        }
    }
void Channel::set_read_callback(std::function<void()> _callback) {
    read_callback = _callback;
}
void Channel::set_use_threadPool(bool use) { use_threadPool = use; }