#include "eventLoop.h"

#include <vector>

#include "channel.h"
#include "epoll.h"

EventLoop::EventLoop() : ep(new Epoll()), quit(false) {}

EventLoop::~EventLoop() { delete ep; }

void EventLoop::loop() {
    while (!quit) {
        std::vector<Channel *> active_channels = ep->poll();
        for (auto it : active_channels) {
            it->handleEvent();
        }
    }
}

void EventLoop::update_channel(Channel *ch) { ep->update_channel(ch); }