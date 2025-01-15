#include "socket.h"

#include <arpa/inet.h>
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
    // if (read_bytes == -1) {
    //     if (errno == EINTR) {  // 客户端正常中断、继续读取
    //                            // printf("continue reading");
    //                            // 如果是被信号中断，继续读取
    //     } else if ((errno == EAGAIN) ||
    //                (errno ==
    //                 EWOULDBLOCK)) {  // 非阻塞IO，这个条件表示数据全部读取完毕
    //         // std::cerr << "finish reading once, errno: " <<
    //         // std::strerror(errno) << std::endl;
    //     } else {
    //         // std::cerr << "Error reading from socket: " <<
    //         // std::strerror(errno) << std::endl; close();
    //     }
    // }
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
    DEBUG_PRINT("accept():new client fd %d! IP: %s Port: %d\n", client_sockfd,
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

// 一般由客户端调用
void Socket::connect(InetAddress *addr) {
    struct sockaddr_in serv_addr = addr->addr;

    // 判断是否为非阻塞
    int flags = fcntl(sockfd, F_GETFL, 0);
    bool isNonBlocking = (flags & O_NONBLOCK);

    if (isNonBlocking) {
        while (true) {
            int ret = ::connect(sockfd, (struct sockaddr *)&serv_addr,
                                sizeof(serv_addr));
            if (ret == 0) {
                // 连接成功
                break;
            } else if (ret == -1 && errno == EINPROGRESS) {
                // 使用 select 等待可写，以确定连接是否完成
                fd_set wset;
                FD_ZERO(&wset);
                FD_SET(sockfd, &wset);
                struct timeval tv;
                tv.tv_sec = 5;  // 例如 5 秒超时
                tv.tv_usec = 0;

                ret = select(sockfd + 1, nullptr, &wset, nullptr, &tv);
                if (ret <= 0) {
                    // 超时或错误
                    perror("select");
                    break;
                }

                if (FD_ISSET(sockfd, &wset)) {
                    int so_error = 0;
                    socklen_t len = sizeof(so_error);
                    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
                    if (so_error == 0) {
                        // 连接成功
                        break;
                    } else {
                        // 连接失败
                        fprintf(stderr, "socket connect error: %s\n",
                                strerror(so_error));
                        break;
                    }
                }
            } else {
                // 其他错误
                perror("socket connect error");
                break;
            }
        }
    } else {
        // 阻塞式连接
        if (::connect(sockfd, (struct sockaddr *)&serv_addr,
                      sizeof(serv_addr)) == -1) {
            perror("socket connect error");
        }
    }
}

InetAddress::InetAddress() : addr_len(sizeof(addr)) {
    bzero(&addr, sizeof(addr));
}
// 默认是ipv4
InetAddress::InetAddress(const char *ip, uint16_t port)
    : addr_len(sizeof(addr)) {
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    addr_len = sizeof(addr);
}

InetAddress::~InetAddress() {}
