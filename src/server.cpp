#include "util/server.h"

#include <iostream>
#include <memory>

#include "util/eventLoop.h"
#include "util/socket.h"
#include "util/tcpConnection.h"
#include "util/util.h"
int main() {
    Server *server = new Server(std::make_unique<InetAddress>("127.0.0.1", 7777), HANDLE_ECHO);
    load_env_file("/home/fz/cpp-project/cpp-server/.env");

    std::cout << "Server is running on" << std::endl;
    // 服务器开始监听
    server->start_work();
    delete server;
    return 0;
}