#include"util/util_socket.h"
#include"error/socket_exception.h"
#include<sys/socket.h>
#include<arpa/inet.h>
#include<cstring>
#include<iostream>
#include<unistd.h>
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

        struct sockaddr_in client_addr;
        bzero(&client_addr, sizeof(client_addr));

        int client_sockfd = acceptConnection(sockfd, client_addr);
        readFromClient(client_sockfd);

        close(sockfd);
    } catch (const SocketException& e) {
        std::cerr << "Socket error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}