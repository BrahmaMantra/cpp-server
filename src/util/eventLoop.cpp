#include "eventLoop.h"

#include <vector>

#include "channel.h"
#include "epoll.h"

EventLoop::EventLoop()
    : ep(new Epoll()), thread_pool(new ThreadPool), quit(false) {}

EventLoop::~EventLoop() {
    delete ep;
    delete thread_pool;
}

void EventLoop::loop() {
    while (!quit) {
        std::vector<Channel *> active_channels = ep->poll();
        for (auto it : active_channels) {
            it->handleEvent();
        }
    }
}

void EventLoop::update_channel(Channel *ch) { ep->update_channel(ch); }

void EventLoop::add_task(std::function<void()> task) { thread_pool->submit(task); }