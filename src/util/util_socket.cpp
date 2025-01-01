#include"util_socket.h"
#include<error/socket_exception.h>
#include<stdlib.h>
#include <sys/socket.h>
#include<iostream>
#include<arpa/inet.h>
#include<unistd.h>
#include<netinet/in.h>
void errif(bool condition,const char *errmsg){
    if(condition){
        std::cerr<<errmsg<<std::endl;
        exit(EXIT_FAILURE);
    }
}

void writeToSocket(int sockfd, const char* buf, size_t len) {
    ssize_t write_bytes = write(sockfd, buf, len);
    std::cout << "write_bytes is " << write_bytes << std::endl;
    if (write_bytes == -1) {
        handleError(SocketError::SEND_FAILED);
    }
}

void readFromSocket(int sockfd, char* buf, size_t len) {
    ssize_t read_bytes = read(sockfd, buf, len);
    if (read_bytes > 0) {
        std::cout << "message from server is: " << buf << std::endl;
    } else if (read_bytes == 0) {
        std::cout << "server socket disconnected!\n";
        close(sockfd);
        exit(0);
    } else if (read_bytes == -1) {
        handleError(SocketError::RECEIVE_FAILED);
    }
}
void bindSocket(int sockfd, const struct sockaddr_in& serv_addr) {
    if (bind(sockfd, reinterpret_cast<const struct sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1) {
        handleError(SocketError::BIND_FAILED);
    }
}

void listenSocket(int sockfd) {
    if (listen(sockfd, SOMAXCONN) == -1) {
        handleError(SocketError::LISTEN_FAILED);
    }
}
int acceptConnection(int sockfd, struct sockaddr_in& client_addr) {
    socklen_t client_addr_len = sizeof(client_addr);
    int client_sockfd = accept(sockfd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
    if (client_sockfd == -1) {
        handleError(SocketError::ACCEPT_FAILED);
    }
    printf("new client fd %d! IP: %s Port: %d\n", client_sockfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    return client_sockfd;
}
// 目前是阻塞且单线程的，后续优化
void readFromClient(int client_sockfd) {
    char buf[1024];
    while (true) {
        bzero(buf, sizeof(buf));
        ssize_t read_bytes = read(client_sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            std::cout << "message from client fd " << client_sockfd << ": " << buf << std::endl;
            write(client_sockfd, buf, sizeof(buf)); // echo服务
        } else if (read_bytes == 0) {
            std::cout << "client fd " << client_sockfd << " is disconnected" << std::endl;
            close(client_sockfd);
            break;
        } else if (read_bytes == -1) {
            if (errno == EINTR) {
                continue; // 如果是被信号中断，继续读取
            } else {
                close(client_sockfd);
                handleError(SocketError::RECEIVE_FAILED);
            }
        }
    }
}