#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <iostream>
#include <thread>

#include "error/socket_exception.h"
#include "util/socket.h"
#include "util/util.h"
#define BUFFER_SIZE 1024

void receive_messages(Socket *client_sock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        bzero(&buffer, sizeof(buffer));
        ssize_t read_bytes = client_sock->read(buffer, sizeof(buffer));
        if (read_bytes > 0) {
            std::string message(buffer, read_bytes);
            std::cout << "\nReceived message: " << message << std::endl;
            if (message == HEARTBEAT_MSG) {
                const char *response = HEARTBEAT_RESPONSE;
                std::cout << "\nSend message: " << response << std::endl;
                client_sock->write(response, strlen(response));
            } else {
                printf("message from server: %s\n", buffer);
            }
        } else if (read_bytes == 0) {
            printf("server socket disconnected!\n");
            break;
        } else if (read_bytes == -1) {
            client_sock->close();
            errif(true, "socket read error");
        }
    }
}

int main() {
    Socket *client_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 7777);

    client_sock->connect(serv_addr);

    std::thread receiver(receive_messages, client_sock);
    receiver.detach();

    while (true) {
        char buf[BUFFER_SIZE];
        bzero(&buf, sizeof(buf));
        std::cin >> buf;
        if (strncmp(buf, "exit", 4) == 0) {
            break;
        }
        client_sock->write(buf, sizeof(buf));
    }

    client_sock->close();
    return 0;
}