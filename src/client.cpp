#include"util/socket.h"
#include "error/socket_exception.h"
#include<sys/socket.h>
#include<arpa/inet.h>
#include<iostream>
#include<unistd.h>

#define BUFFER_SIZE 1024
int main() {
        // int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        // if (sockfd == -1) {
        //     handleError(SocketError::CONNECTION_FAILED);
        // }

        // struct sockaddr_in serv_addr;
        // bzero(&serv_addr, sizeof(serv_addr));
        // serv_addr.sin_family = AF_INET;
        // serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        // serv_addr.sin_port = htons(8888);

        Socket * client_sock = new Socket();
        InetAddress * serv_addr = new InetAddress("127.0.0.1", 8888);
        

        client_sock->connect(serv_addr);

        while (true) {
            char buf[BUFFER_SIZE];
            bzero(buf, sizeof(buf));
            std::cin >> buf;
            client_sock->write(buf, sizeof(buf));
            bzero(buf, sizeof(buf));
            client_sock->read(buf, sizeof(buf));
        }

        client_sock->close();
    return 0;
    }