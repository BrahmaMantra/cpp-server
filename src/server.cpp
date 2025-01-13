#include "util/server.h"

#include <iostream>
#include <memory>

#include "util/eventLoop.h"
#include "util/socket.h"
#include "util/tcpConnection.h"
#include "util/util.h"
int main() {
    load_env_file(".env");
    std::unique_ptr<EventLoop> loop = std::make_unique<EventLoop>(1);
    Server *server = new Server(
        std::move(loop), std::make_unique<InetAddress>("127.0.0.1", 7777));

    std::cout << "Server is running on" << std::endl;
    server->on_connect(server_handlers[HANDLE_ECHO]);
    // 服务器开始监听
    server->start_work();
    delete server;
    return 0;
}