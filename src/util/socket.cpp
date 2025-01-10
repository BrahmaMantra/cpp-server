#include "socket.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>

#include "error/socket_exception.h"
#include "util.h"

// 默认IPV4，TCP
Socket::Socket() : sockfd(-1) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");
}
Socket::Socket(int _fd) : sockfd(_fd) {
    errif(sockfd == -1, "socket create error");
}
Socket::~Socket() {
    if (sockfd != -1) {
        close();
        sockfd = -1;
    }
}

void Socket::bind(InetAddress *addr) {
    if (::bind(sockfd, (sockaddr *)&addr->addr, addr->addr_len) == -1) {
        std::cerr << "Error binding socket: " << std::strerror(errno)
                  << std::endl;
        handleError(SocketError::BIND_FAILED);
    }
}

void Socket::listen() {
    if (::listen(sockfd, SOMAXCONN) == -1) {
        std::cerr << "Error listening on socket: " << std::strerror(errno)
                  << std::endl;
        handleError(SocketError::LISTEN_FAILED);
    }
}
ssize_t Socket::write(const char *buf, size_t len) {
    ssize_t write_bytes = ::write(sockfd, buf, len);
    // std::cout << "write_bytes is " << write_bytes << std::endl;
    if (write_bytes == -1) {
        std::cerr << "Error writing to socket: " << std::strerror(errno)
                  << std::endl;
        handleError(SocketError::SEND_FAILED);
    }
    return write_bytes;
}

// read坚决不close，close由上层调用
ssize_t Socket::read(char *buf, size_t len) {
    ssize_t read_bytes = ::read(sockfd, buf, len);
    if (read_bytes == -1) {
        if (errno == EINTR) {  // 客户端正常中断、继续读取
                               // printf("continue reading");
                               // 如果是被信号中断，继续读取
        } else if ((errno == EAGAIN) ||
                   (errno ==
                    EWOULDBLOCK)) {  // 非阻塞IO，这个条件表示数据全部读取完毕
            // std::cerr << "finish reading once, errno: " <<
            // std::strerror(errno) << std::endl;
        } else {
            // std::cerr << "Error reading from socket: " <<
            // std::strerror(errno) << std::endl; close();
        }
    }
    return read_bytes;
}

int Socket::accept(InetAddress *client_addr) {
    
    int client_sockfd = ::accept(sockfd, (sockaddr *)&client_addr->addr,
                                 &client_addr->addr_len);
    if (client_sockfd == -1) {
        std::cerr << "Error accepting connection: " << std::strerror(errno)
                  << std::endl;
        handleError(SocketError::ACCEPT_FAILED);
    }
    printf("new client fd %d! IP: %s Port: %d\n", client_sockfd,
           inet_ntoa(client_addr->addr.sin_addr),
           ntohs(client_addr->addr.sin_port));
    return client_sockfd;
}

int Socket::close() {
    if (sockfd == -1) {
        std::cerr << "reclose socket: " << std::endl;
        return -1;
    }
    return ::close(sockfd);
}

void Socket::setNonblocking() {
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);
}
int Socket::get_fd() { return sockfd; }
void Socket::connect(InetAddress *addr) {
    if (::connect(sockfd, (sockaddr *)&addr->addr, addr->addr_len) == -1) {
        std::cerr << "Error connecting to server: " << std::strerror(errno)
                  << std::endl;
        handleError(SocketError::CONNECTION_FAILED);
    }
}
