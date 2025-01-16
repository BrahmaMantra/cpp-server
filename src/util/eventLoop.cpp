#include "eventLoop.h"

#include <cstring>
#include <string>
#include <vector>

#include "channel.h"
#include "epoll.h"
#include "socket.h"
#include "util.h"
EventLoop::EventLoop(int loop_id) : quit(false), loop_id(loop_id) {
    ep = std::make_unique<Epoller>();
}

EventLoop::~EventLoop() {}

void EventLoop::loop() const {
    while (!quit) {
        std::vector<Channel *> active_channels = ep->poll(5000);
        for (auto it : active_channels) {
            INFO_PRINT("loop(): loop_id: %d, fd: %d prepared to work \n",
                        loop_id, it->get_fd());

            it->handleEvent();
        }
    }
}

// 每次更新了epoll(Channel)事件后，都要调用这个函数
// 也是进入epollLoop的唯一入口
void EventLoop::update_channel(std::shared_ptr<Channel> ch) {
        ep->update_channel(std::weak_ptr<Channel>(ch));
        ch.reset();
}

// 真正的close_callback
void EventLoop::delete_connection(Socket *client_sock) {
    int fd = client_sock->get_fd();
    ep->_delete_fd_in_channels(fd);
    ep->_delete_connection(client_sock);
}
void EventLoop::insert_connection(int fd, std::shared_ptr<TcpConnection> connection) {
    ep->_insert_connection(fd, std::move(connection));
}
std::shared_ptr<TcpConnection> EventLoop::find_connection(int id) {
    return ep->_find_connection(id);
}