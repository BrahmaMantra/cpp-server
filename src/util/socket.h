#pragma once

#include <cstddef>
#include"InetAddress.h"
#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void errif(bool,const char*);

class InetAddress;
class Socket{
private:
    int sockfd;
public:
    Socket();
    Socket(int);
    ~Socket();
    void bind(InetAddress *addr);
    void listen();
    void setnonblocking();
    int accept(InetAddress *client_addr);
    int getFd();
    void connect(InetAddress *addr);
    ssize_t write(const char* buf, size_t len);

    ssize_t read(char* buf, size_t len);
    int close();
};
void echoServer(Socket *client_sockfd);