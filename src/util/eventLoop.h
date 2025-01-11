#pragma once
#include"threadPool/threadPool.h"
class Epoll;
class Channel;
class EventLoop {
   private:
    Epoll *ep;
    bool quit;
    int loop_id;
    
   public:
    EventLoop(int);
    ~EventLoop();

    void loop();
    void update_channel(Channel *);
};