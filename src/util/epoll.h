#pragma once
#include <sys/epoll.h>

#include <vector>

#include "channel.h"
class Epoll {
   private:
    int epfd;
    struct epoll_event *events;

   public:
    Epoll();
    ~Epoll();

    void add_fd(int fd, uint32_t op);
    void update_channel(Channel *);
    std::vector<Channel *> poll(int timeout = -1);
};
