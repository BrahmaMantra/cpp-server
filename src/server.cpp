#include "Epoll.h"
#include "error/socket_exception.h"
#include "util/socket.h"
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
  Socket *serv_sock = new Socket();
  InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
  serv_sock->bind(serv_addr);
  serv_sock->listen();

  Epoll *ep = new Epoll();
  serv_sock->setnonblocking();
  // EPOLLIN可读，EPOLLET边缘触发
  ep->addFd(serv_sock->getFd(), EPOLLIN | EPOLLET);
  while (true) {
    std::vector<epoll_event> events = ep->poll();
    int nfds = events.size();
    // 处理所有发生的事件
    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == serv_sock->getFd()) { // 新客戶端连接
        InetAddress *clnt_addr =
            new InetAddress(); // 会发生内存泄露！没有delete
        Socket *clnt_sock = new Socket(
            serv_sock->accept(clnt_addr)); // 会发生内存泄露！没有delete
        clnt_sock->setnonblocking();
        cout<<"new client connected, fd is "<<clnt_sock->getFd()<<endl;
        ep->addFd(clnt_sock->getFd(), EPOLLIN | EPOLLET);
      } else if (events[i].events & EPOLLIN) { // 可读事件
        Socket *client_sock = new Socket(events[i].data.fd);
        echoServer(client_sock);
      } else {
        std::cout << "TODO!" << std::endl;
      }
    }
  }
  delete serv_sock;
  delete serv_addr;
  return 0;
}