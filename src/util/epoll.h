#pragma once
#include <sys/epoll.h>

#include <vector>

#include "channel.h"
// 用来管理channel的类
class Epoller {
   private:
    int epfd;
    // epoll_wait返回的事件数组，最大事件数
    struct epoll_event *events;

   public:
    Epoller();
    ~Epoller();

    void add_fd(int fd, uint32_t op);
    void update_channel(Channel *) const;
    void delete_channel(Channel *) const;
    std::vector<Channel *> poll(int timeout = -1);
};
