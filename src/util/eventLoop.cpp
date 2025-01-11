#include "eventLoop.h"

#include <cstring>
#include <string>
#include <vector>

#include "channel.h"
#include "epoll.h"
#include "util.h"
EventLoop::EventLoop(int loop_id)
    : ep(new Epoll()), quit(false), loop_id(loop_id) {}

EventLoop::~EventLoop() { delete ep; }

void EventLoop::loop() {
    while (!quit) {
        std::vector<Channel *> active_channels = ep->poll();
        for (auto it : active_channels) {
            std::string debug_info = "loop(): loop_id: " + std::to_string(loop_id) +
                             ", prepare to work";
            debugPrint(debug_info);
            it->handleEvent();
        }
    }
}

void EventLoop::update_channel(Channel *ch) { ep->update_channel(ch); }
