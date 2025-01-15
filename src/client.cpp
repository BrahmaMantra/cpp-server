#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <thread>
#include <atomic>

#include "error/socket_exception.h"
#include "util/socket.h"
#include "util/util.h"
#define BUFFER_SIZE 1024


int main() {
    Socket *client_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 7777);

    client_sock->connect(serv_addr);

    while (true) {
        char buf[BUFFER_SIZE];
        bzero(&buf, sizeof(buf));
        std::cin >> buf;
        if (strncmp(buf, "exit", 4) == 0) {
            break;
        }
        client_sock->write(buf, sizeof(buf));
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = client_sock->read(buf, sizeof(buf));
        if (read_bytes > 0) {
            if (strncmp(buf, "PING", 4) == 0) {
                const char *response = "PONG";
                client_sock->write(response, strlen(response));
            } else {
                printf("message from server: %s\n", buf);
            }
        } else if (read_bytes == 0) {
            printf("server socket disconnected!\n");
            break;
        } else if (read_bytes == -1) {
            client_sock->close();
            errif(true, "socket read error");
        }
    }

    client_sock->close();
    return 0;
}