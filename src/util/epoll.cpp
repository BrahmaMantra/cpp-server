#include "epoll.h"

#include <string.h>
#include <unistd.h>

#include <iostream>

#include "channel.h"
#include "socket.h"
#include "util.h"
Epoll::Epoll() : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");
    events = new epoll_event[MAX_EVENTS];
    std::cout << "Epoll(): epfd is " << epfd << ",events is " << events
              << std::endl;
    bzero(events, sizeof(*events) * MAX_EVENTS);
}

Epoll::~Epoll() {
    if (epfd != -1) {
        close(epfd);
        epfd = -1;
    }
    delete[] events;
}

void Epoll::add_fd(int fd, uint32_t op) {
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.fd = fd;
    ev.events = op;
    errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1,
          "epoll add event error");
}

std::vector<Channel *> Epoll::poll(int timeout) {
    std::vector<Channel *> active_channels;
    // 等待事件发生,这个函数会阻塞，直到某个事件发生或者超时
    // std::cout<<"epfd is "<<epfd<<std::endl<<"events is "<<events<<std::endl;
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    errif(nfds == -1, "epoll wait error");
    for (int i = 0; i < nfds; ++i) {
        Channel *channel = static_cast<Channel *>(events[i].data.ptr);
        channel->set_revents(events[i].events);
        active_channels.push_back(channel);
    }
    return active_channels;
}

void Epoll::update_channel(Channel *channel) {
    int fd = channel->get_fd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->get_events();
    if (!channel->is_in_epoll()) {
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->set_in_epoll();
        // std::cout<<"Epoll: add Channel to epoll tree success, the Channel's
        // fd is: "<<fd<<",events is "<<events<<std::endl;
    } else {
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1,
              "epoll modify error");
        // std::cout<<"Epoll: modify Channel in epoll tree success, the
        // Channel's fd is: "<<fd<<std::endl;
    }
}