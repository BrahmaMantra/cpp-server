#include "error/socket_exception.h"
#include "util/util_socket.h"
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

void setnonblocking(int fd) {
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}
int main() {
  try {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
      handleError(SocketError::CONNECTION_FAILED);
    }

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

    bindSocket(sockfd, serv_addr);
    listenSocket(sockfd);

    // 创建epoll文件描述符
    int epfd = epoll_create(MAX_EVENTS);
    errif(epfd == -1, "epoll create error");

    // 创建epoll事件队列
    struct epoll_event events[MAX_EVENTS], ev;
    bzero(events, sizeof(events));
    bzero(&ev, sizeof(ev));

    // EPOLLIN可读，EPOLLET边缘触发
    ev.data.fd = sockfd;
    ev.events = EPOLLIN | EPOLLET;
    setnonblocking(sockfd);

    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    while (true) {
      // 等待事件发生
      int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
      errif(nfds == -1, "epoll wait error");
      // 处理所有发生的事件
      for (int i = 0; i < nfds; i++) {
        if (events[i].data.fd == sockfd) { // 新客戶端连接
          struct sockaddr_in client_addr;
          bzero(&client_addr, sizeof(client_addr));
          socklen_t client_addr_len = sizeof(client_addr);

          // 接收新客户端的连接
          int client_sockfd = acceptConnection(sockfd, client_addr);

          // 配置新客户端的epoll事件
          bzero(&ev, sizeof(ev));
          ev.data.fd = client_sockfd;
          ev.events = EPOLLIN | EPOLLET;
          setnonblocking(client_sockfd);
          epoll_ctl(epfd, EPOLL_CTL_ADD, client_sockfd, &ev);
        } else if (events[i].events & EPOLLIN) { // 可读事件
          readFromClient(events[i].data.fd);
        } else {
          std::cout << "TODO!" << std::endl;
        }
      }
    }
    close(sockfd);
  } catch (const SocketException &e) {
    std::cerr << "Socket error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}