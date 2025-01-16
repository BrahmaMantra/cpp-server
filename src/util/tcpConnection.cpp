#include "tcpConnection.h"

#include <fcntl.h>
#include <string.h>

#include <cassert>
#include <memory>

#include "buffer.h"
#include "channel.h"
#include "server.h"
#include "socket.h"
#include "util.h"
#define READ_BUFFER 1024

TcpConnection::TcpConnection(EventLoop *loop, Socket *_client_sock)
    : loop(loop), client_sock(_client_sock) {
    // if (loop != nullptr) {
    //     channel = std::make_unique<Channel>(loop, client_sock->get_fd());
    //     channel->enable_read();
    //     channel->enable_ET();
    // }
    channel = std::make_shared<Channel>(loop, client_sock->get_fd());

    read_buffer_ = std::make_unique<Buffer>();
    send_buffer_ = std::make_unique<Buffer>();

    channel->set_close_callback([this]() {
        DEBUG_PRINT("TcpConnection::TcpConnection(): close_callback\n");
        handle_close();
    });
    channel->set_read_callback([this]() { handle_recv(); });
    state = ConnectionState::Connected;
}
TcpConnection::~TcpConnection() { 
    delete client_sock; 
    client_sock = nullptr;
    }

void TcpConnection::read(bool is_ping_test) {
    assert(state == ConnectionState::Connected);
    if (client_sock->get_fd() == -1) {
        std::cerr << "client_sock->get_fd() == -1" << std::endl;
        return;
    }
    std::lock_guard<std::mutex> lock(buffer_mtx);
    read_buffer_->clear();
    if (is_ping_test) {
        // 尝试读取 "Pong" 消息,超时时间为5s
        read_blocking(5);
        if (read_buffer_->size() <= 0) {
            printf("handle_HEARTBEAT_MSG():  fd:%d is die!\n",
                        client_sock->get_fd());
            loop->delete_connection(client_sock);
            return;
        }
    } else {
        read_nonBlocking();
        DEBUG_PRINT("read(): message from client fd %d: %s\n",
                    client_sock->get_fd(), read_buffer_->c_str());
    }
}

void TcpConnection::write() {
    if(state != ConnectionState::Connected) {
        std::cerr << "write(): state is not connected" << std::endl;
        return;
    }
    // assert(state == ConnectionState::Connected);
    write_nonBlocking();
    send_buffer_->clear();
}
void TcpConnection::send(const std::string &msg) {
    send_buffer_->clear();
    send_buffer_->append(msg.c_str(), msg.size());
    write();
}

void TcpConnection::handle_close() {
    if (state != ConnectionState::Closed) {
        state = ConnectionState::Closed;
    }
    if (close_callback) {
        close_callback(client_sock);
    } else {
        std::cerr << "close_callback is not set" << std::endl;
    }
}

void TcpConnection::handle_recv() {
    if (state != ConnectionState::Connected) {
        std::cout << "handle_recv(): state is not connected" << std::endl;
        state = ConnectionState::Connected;
    }
    if (recv_callback) {
        recv_callback(this);
    } else {
        std::cerr << "recv_callback is not set,sockfd = "
                  << client_sock->get_fd() << std::endl;
        recv_callback = hanele_echo;
        recv_callback(this);
    }
}

void TcpConnection::read_nonBlocking() {
    // 设置socket为非阻塞模式

    int flags = fcntl(client_sock->get_fd(), F_GETFL, 0);
    fcntl(client_sock->get_fd(), F_SETFL, flags | O_NONBLOCK);
    char buf[READ_BUFFER];
    // 使用非阻塞io读取客户端buffer，一次读取buf大小数据知道读取完毕
    while (1) {
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = client_sock->read(buf, sizeof(buf));
        if (read_bytes > 0) {
            read_buffer_->append(buf, read_bytes);
        } else if (read_bytes == 0) {
            DEBUG_PRINT("READ EOF,client fd %d is disconnected\n",
                        client_sock->get_fd());
            client_sock->close();
            break;
        } else if (read_bytes == -1 &&
                   errno == EINTR) {  //客户端正常中断、继续读取
            DEBUG_PRINT("errno == EINTR,continue reading \n");
            continue;
        } else if (read_bytes == -1 &&
                   ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  //非阻塞IO
            DEBUG_PRINT("errno == EAGAIN,finish reading once\n");
            break;
        } else {
            DEBUG_PRINT("read_nonBlocking(): message from client fd %d: %s\n",
                        client_sock->get_fd(),
                        std::string(buf).substr(0, read_bytes).c_str());
            state = ConnectionState::Closed;
            break;
        }
    }
}

void TcpConnection::read_blocking(int timeout_sec) {
    // 设置socket为阻塞模式
    int flags = fcntl(client_sock->get_fd(), F_GETFL, 0);
    fcntl(client_sock->get_fd(), F_SETFL, flags & ~O_NONBLOCK);

    // 设置读取超时时间
    struct timeval timeout;
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;
    setsockopt(client_sock->get_fd(), SOL_SOCKET, SO_RCVTIMEO, &timeout,
               sizeof(timeout));

    char buf[READ_BUFFER];
    while (1) {
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = client_sock->read(buf, sizeof(buf));
        if (read_bytes > 0) {
            read_buffer_->append(buf, read_bytes);
            break;
        } else if (read_bytes == 0) {
            printf("READ EOF,client fd %d is disconnected\n",
                   client_sock->get_fd());
            client_sock->close();
            break;
        } else if (read_bytes == -1 &&
                   errno == EINTR) {  // 客户端正常中断、继续读取
            printf("errno == EINTR,continue reading \n");
            continue;
        } else if (read_bytes == -1 && errno == EWOULDBLOCK) {  // 超时
            printf("read_blocking(): timeout on client fd %d\n",
                   client_sock->get_fd());
            break;
        } else {
            printf("read_blocking(): message from client fd %d: %s\n",
                   client_sock->get_fd(),
                   std::string(buf).substr(0, read_bytes).c_str());
            state = ConnectionState::Closed;
            break;
        }
    }
}

void TcpConnection::write_nonBlocking() {
    int sockfd = client_sock->get_fd();
    char buf[send_buffer_->size()];
    memcpy(buf, send_buffer_->c_str(), send_buffer_->size());
    int data_size = send_buffer_->size();
    int data_left = data_size;
    while (data_left > 0) {
        ssize_t write_bytes =
            client_sock->write(buf + data_size - data_left, data_left);
        if (write_bytes == -1 && errno == EINTR) {
            // printf("continue writing\n");
            continue;
        }
        if (write_bytes == -1 && errno == EAGAIN) {
            break;
        }
        if (write_bytes == -1) {
            printf("Other error on client fd %d\n", sockfd);
            state = ConnectionState::Closed;
            handle_close();
            break;
        }
        data_left -= write_bytes;
    }
}
TcpConnection::ConnectionState TcpConnection::get_state() { return state; }
void TcpConnection::set_send_buffer(const char *str) {
    send_buffer_->set_buf(str);
}
const char *TcpConnection::read_buffer() { return read_buffer_->c_str(); }
const char *TcpConnection::send_buffer() { return send_buffer_->c_str(); }

void TcpConnection::set_close_callback(std::function<void(Socket *)> cb) {
    close_callback = cb;
}

// Connection唯一往channel注册回调的函数,决定权交给Server
// 因为Server需要根据具体的业务逻辑来决定如何处理接收到的数据
void TcpConnection::set_recv_callback(std::function<void(TcpConnection *)> cb) {
    recv_callback = cb;
}

Socket *TcpConnection::get_socket() { return client_sock; }

void TcpConnection::init(Server *server) {
    std::function<void(Socket *)> close_callback =
        [this](Socket *_client_sock) { loop->delete_connection(_client_sock); };
    set_close_callback(close_callback);
    std::function<void(TcpConnection *)> _recv_callback =
        server_handlers[server->get_handle_type()];
    set_recv_callback(_recv_callback);  // 来自Server的回调函数
    if (loop != nullptr) {
        channel->enable_read();
        loop->insert_connection(client_sock->get_fd(), shared_from_this());
        loop->update_channel(std::move(channel));
    }
}
