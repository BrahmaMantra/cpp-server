#pragma once
#include "threadPool/threadPool.h"
#include "util.h"
class Epoller;
class Channel;
class EventLoop {
   private:
    std::unique_ptr<Epoller> ep;
    bool quit;  // 退出标志，目前没有用到
    int loop_id;

   public:
    DISALLOW_COPY_AND_MOVE(EventLoop);
    EventLoop(int);
    ~EventLoop();

    void loop() const;
    void update_channel(Channel *) const;
    void delete_channel(Channel *ch) const;
};