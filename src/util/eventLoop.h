#pragma once
#include"threadPool/threadPool.h"
class Epoll;
class Channel;
class EventLoop {
   private:
    Epoll *ep;
    ThreadPool *thread_pool;
    bool quit;
    
   public:
    EventLoop();
    ~EventLoop();

    void loop();
    void update_channel(Channel *);
    void add_task(std::function<void()> task);
};