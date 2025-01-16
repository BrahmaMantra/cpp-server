#include "epoll.h"

#include <string.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#include "channel.h"
#include "socket.h"
#include "util.h"

Epoller::Epoller() : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    connections.clear();
    channels.clear();
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
        auto fds_vec = send_HEARTBEAT_to_all_connections();
        for (auto fd : fds_vec) {
            auto conn = _find_connection(fd);
            if (conn) {
                bool is_ping_test = true;
                conn->read(is_ping_test);
                conn.reset();
            }
        }
        // 检查哪些文件描述符没有活跃，记录并打印
        std::vector<int> inactive_fds;
        for (auto it = channels.begin(); it != channels.end(); ++it) {
            auto channel = it->second;
            // if (channel->get_ready_events() == 0) {
            inactive_fds.push_back(channel->get_fd());
            // }
        }

        // 输出未活动的文件描述符
        if (!inactive_fds.empty()) {
            std::cout << "Inactive file descriptors: ";
            for (int fd : inactive_fds) {
                std::cout << fd << " ";
            }
            std::cout << std::endl;
        }
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

void Epoller::update_channel(std::weak_ptr<Channel> ch) {
    if (auto channel = ch.lock()) {
        int fd = channel->get_fd();
        DEBUG_PRINT("update_channel(): fd = %d \n", fd);
        struct epoll_event ev;
        bzero(&ev, sizeof(ev));
        // 这样在事件发生的时候，我们可以通过这个指针找到对应的Channel
        ev.data.ptr = channel.get();
        ev.events = channel->get_listen_events();

        std::lock_guard<std::mutex> lock(channels_mtx);  // 加锁
        if (channels.find(channel->get_fd()) == channels.end()) {
            channels.insert(std::make_pair(channel->get_fd(), channel));
        }
        if (!channel->is_in_epoll()) {
            errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1,
                  "epoll add error");
            channel->set_in_epoll();
        } else {
            errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1,
                  "epoll modify error");
        }
    } else {
        std::cerr << "Epoller::UpdateChannel channel is expired" << std::endl;

    }
}

// 确保进入这个函数的时候，拿到了channels的锁
void Epoller::delete_channel(std::shared_ptr<Channel> channel) {
    int sockfd = channel->get_fd();
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, nullptr) == -1) {
        std::cout << "Epoller::UpdateChannel epoll_ctl_del failed" << std::endl;
    }
    channel->set_in_epoll(false);
    channel.reset();
}

// 新增方法：发送Ping消息给所有连接
std::vector<int> Epoller::send_HEARTBEAT_to_all_connections() {
    std::vector<int> fds;
    std::lock_guard<std::mutex> lock(connections_mutex);
    for (auto it = connections.begin(); it != connections.end(); ++it) {
        auto conn = it->second;
        if (conn) {
            conn->send(HEARTBEAT_MSG);
            // bool is_ping_test = true;
            // conn->read(is_ping_test);
            fds.push_back(it->first);
        }
    }
    return fds;
}

// 插入或更新连接
void Epoller::_insert_connection(int id,
                                 std::shared_ptr<TcpConnection> connection) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    // auto _connections = connections.get();
    if (connections.find(id) != connections.end()) {
        std::cout << "_insert_connection(): id " << id << " already exists"
                  << std::endl;
    }
    connections[id] = connection;
}
void Epoller::_delete_fd_in_channels(int fd) {
    std::lock_guard<std::mutex> lock(channels_mtx);
    auto it = channels.find(fd);
    if (it == channels.end()) {
        std::cerr << "delete_channel(): fd " << fd << " not found" << std::endl;
        return;
    }
    delete_channel(it->second);  // fd被close后，epoll会自动删除
    channels.erase(it);
    INFO_PRINT("delete_channel(): channel_fd:%d is erase\n", fd);
}

void Epoller::_delete_connection(Socket *client_sock) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    auto it = connections.find(client_sock->get_fd());
    if (it == connections.end()) {
        printf("_delete_connection():client fd %d not found \n",
               client_sock->get_fd());
        return;
    }
    // shared_ptr就不用判断是否为空了
    // 这里会去析构shared_ptr
    connections.erase(client_sock->get_fd());
}

// 查找连接
std::shared_ptr<TcpConnection> Epoller::_find_connection(int id) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    auto it = connections.find(id);
    return it != connections.end() ? it->second : nullptr;
}