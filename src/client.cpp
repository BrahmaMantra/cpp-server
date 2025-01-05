#include"util/util_socket.h"
#include "error/socket_exception.h"
#include<sys/socket.h>
#include<arpa/inet.h>
#include<iostream>
#include<unistd.h>

#define BUFFER_SIZE 1024
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

        if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
            handleError(SocketError::CONNECTION_FAILED);
        }

        while (true) {
            char buf[BUFFER_SIZE];
            bzero(buf, sizeof(buf));
            std::cin >> buf;
            writeToSocket(sockfd, buf, sizeof(buf));

            bzero(buf, sizeof(buf));
            readFromSocket(sockfd, buf, sizeof(buf));
        }

        close(sockfd);
    } catch (const SocketException& e) {
        std::cerr << "Socket error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}