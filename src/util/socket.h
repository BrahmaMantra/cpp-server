#pragma once

#include <netinet/in.h>

#include <cstddef>
#include <iostream>

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

class InetAddress {
   public:
    struct sockaddr_in addr;
    socklen_t addr_len;
    InetAddress();
    InetAddress(const char *ip, uint16_t port);
    ~InetAddress();
};
class Socket {
   private:
    int sockfd;

   public:
    Socket();
    Socket(int);
    ~Socket();
    void bind(InetAddress *addr);
    void listen();
    void setNonblocking();
    int accept(InetAddress *client_addr);
    int get_fd();
    void connect(InetAddress *addr);
    ssize_t write(const char *buf, size_t len);

    ssize_t read(char *buf, size_t len);
    int close();
};