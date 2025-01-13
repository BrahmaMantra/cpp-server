#include "tcpConnection.h"

#include <string.h>

#include <cassert>
#include <memory>

#include "buffer.h"
#include "channel.h"
#include "socket.h"
#include "util.h"
#define READ_BUFFER 1024

TcpConnection::TcpConnection(EventLoop *loop, Socket *_client_sock)
    : loop(loop), client_sock(_client_sock) {
    if (loop != nullptr) {
        channel = std::make_unique<Channel>(loop, client_sock->get_fd());
        channel->enable_read();
        channel->enable_ET();
    }
    read_buffer_ = std::make_unique<Buffer>();
    send_buffer_ = std::make_unique<Buffer>();
    channel->set_close_callback([this]() {
        DEBUG_PRINT("TcpConnection::TcpConnection(): close_callback\n");
        handle_close();
    });
    state = ConnectionState::Connected;
}
TcpConnection::~TcpConnection() { delete client_sock; }

void TcpConnection::read() {
    assert(state == ConnectionState::Connected);
    read_buffer_->clear();
    read_nonBlocking();
    DEBUG_PRINT("read(): message from client fd %d: %s\n",
                client_sock->get_fd(), read_buffer_->c_str());
}

void TcpConnection::write() {
    assert(state == ConnectionState::Connected);
    write_nonBlocking();
    send_buffer_->clear();
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

void TcpConnection::read_nonBlocking() {
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
                   ((errno == EAGAIN) ||
                    (errno ==
                     EWOULDBLOCK))) {  //非阻塞IO，这个条件表示数据全部读取完毕
            // printf("finish reading once, errno: %d\n", errno);
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
    channel->set_read_callback([this]() {
        read();
        recv_callback(this);
    });
}

Socket *TcpConnection::get_socket() { return client_sock; }