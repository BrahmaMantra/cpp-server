#include "eventLoop.h"

#include <cstring>
#include <string>
#include <vector>

#include "channel.h"
#include "epoll.h"
#include "util.h"
EventLoop::EventLoop(int loop_id) : quit(false), loop_id(loop_id) {
    ep = std::make_unique<Epoller>();
}

EventLoop::~EventLoop() {}

void EventLoop::loop() const {
    while (!quit) {
        std::vector<Channel *> active_channels = ep->poll();
        for (auto it : active_channels) {
            DEBUG_PRINT("loop(): loop_id: %d, fd: %d prepared to work \n",
                        loop_id, it->get_fd());

            it->handleEvent();
        }
    }
}

// 每次更新了epoll(Channel)事件后，都要调用这个函数
// 也是进入epollLoop的唯一入口
void EventLoop::update_channel(Channel *ch) const { ep->update_channel(ch); }
void EventLoop::delete_channel(Channel *ch) const { ep->delete_channel(ch); }
