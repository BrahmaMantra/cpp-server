#include "epoll.h"

#include <string.h>
#include <unistd.h>

#include <iostream>

#include "channel.h"
#include "socket.h"
#include "util.h"
Epoller::Epoller()
    : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    all_connections =std::make_shared<std::unordered_map<int, TcpConnection *>>();
    DEBUG_PRINT("Epoller():epfd = %d \n", epfd);
    errif(epfd == -1, "epoll create error");
    events = new epoll_event[MAX_EVENTS];
    bzero(events, sizeof(*events) * MAX_EVENTS);
}

Epoller::~Epoller() {
    if (epfd != -1) {
        close(epfd);
        epfd = -1;
    }
    delete[] events;
}

void Epoller::add_fd(int fd, uint32_t op) {
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.fd = fd;
    ev.events = op;
    {
        std::lock_guard<std::mutex> lock(mtx);  // 加锁
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1,
              "epoll add event error");
    }
}

std::vector<Channel *> Epoller::poll(int timeout) {
    std::vector<Channel *> active_channels;
    // 等待事件发生,这个函数会阻塞，直到某个事件发生或者超时
    // 这个函数会把返回的事件保存到events数组中，唯一修改events数组的地方
    int nfds;
    // 等待事件发生，处理 EINTR 错误
    while (true) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);

        if (nfds == -1 && errno != EINTR) {  // 处理gdb调试时的中断
            errif(nfds == -1, "epoll wait error");
        }

        // 如果没有被中断并且返回了有效的事件数量，就跳出循环
        if (nfds != -1) {
            break;
        }
    }

    errif(nfds == -1, " epoll wait error");
    if (nfds == 0) {
        send_ping_to_all_connections();
    }
    DEBUG_PRINT("poll():epfd = %d \n", epfd);
    for (int i = 0; i < nfds; ++i) {
        Channel *channel = static_cast<Channel *>(events[i].data.ptr);
        // 把正在发生的事件保存到channel中
        channel->set_ready_events(events[i].events);
        active_channels.emplace_back(channel);
    }
    return active_channels;
}

void Epoller::update_channel(Channel *channel) const {
    int fd = channel->get_fd();
    DEBUG_PRINT("update_channel(): fd = %d \n", fd);
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    // 这样在事件发生的时候，我们可以通过这个指针找到对应的Channel
    ev.data.ptr = channel;
    ev.events = channel->get_listen_events();

    std::lock_guard<std::mutex> lock(mtx);  // 加锁
    if (!channel->is_in_epoll()) {
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->set_in_epoll();
    } else {
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1,
              "epoll modify error");
    }
}
void Epoller::delete_channel(Channel *channel) const {
    int sockfd = channel->get_fd();
    {
        std::lock_guard<std::mutex> lock(mtx);  // 加锁
        if (epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, nullptr) == -1) {
            std::cout << "Epoller::UpdateChannel epoll_ctl_del failed"
                      << std::endl;
        }
        channel->set_in_epoll(false);
    }
    channel->~Channel();
}

    // 新增方法：发送Ping消息给所有连接
    void Epoller::send_ping_to_all_connections() const {
        if (auto connections = all_connections) {
            for (auto& [fd, conn] : *connections) {
                if (conn) {
                    conn->send("Ping");
                }
            }
        }
    }

    // 插入或更新连接
void Epoller::_insert_connection(int id, TcpConnection* connection) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    auto _connections = all_connections.get();
    if (_connections->find(id) != _connections->end()) {
        std::cout << "_insert_connection(): id " << id << " already exists" << std::endl;
    }
    (*_connections)[id] = connection;
}

void Epoller::_delete_connection(Socket *client_sock) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    auto _connections = all_connections.get();
    auto it = _connections->find(client_sock->get_fd());
    if (it == _connections->end()) {
        printf("_delete_connection():client fd %d not found \n", client_sock->get_fd());
        return;
    }
    TcpConnection *conn = (*_connections)[client_sock->get_fd()];
    _connections->erase(client_sock->get_fd());
    // 确认了调用delete conn就会调用TcpConnection的析构函数释放fd
    INFO_PRINT("_delete_connection():client fd %d  is deleted \n", client_sock->get_fd());
    delete conn;
}

// 查找连接
TcpConnection* Epoller::_find_connection(int id) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    auto _connections = all_connections.get();
    auto it = _connections->find(id);
    return it != _connections->end() ? it->second : nullptr;
}

