#ifndef UTIL_H
#define UTIL_H

#include <cstddef>

void errif(bool,const char*);
void writeToSocket(int sockfd, const char* buf, size_t len);
void readFromSocket(int sockfd, char* buf, size_t len);
void bindSocket(int sockfd, const struct sockaddr_in& serv_addr);
void listenSocket(int sockfd);
int acceptConnection(int sockfd, struct sockaddr_in& client_addr);
void readFromClient(int client_sockfd);

#endif