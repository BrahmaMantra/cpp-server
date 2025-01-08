#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "error/socket_exception.h"
#include "util/InetAddress.h"
#include "util/socket.h"
#include "util/util.h"

#define NUM_CLIENTS 100
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7777

std::atomic<int> successful_connections(0);

void client_request(int thread_num) {
    try {
        Socket client_sock;
        InetAddress serv_addr(SERVER_IP, SERVER_PORT);
        client_sock.connect(&serv_addr);

        // 发送消息到服务器
        // std::string message = "Hi, I'm the " + std::to_string(thread_num) + "
        // thread"; client_sock.write(message.c_str(), message.size());

        successful_connections++;
        client_sock.close();
    } catch (const SocketException &e) {
        std::cerr << "Error connecting to server: " << e.what() << std::endl;
    }
}

TEST(ClientRequestsTest, MultipleConnections) {
    std::vector<std::thread> clients;
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        // std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 延迟
        clients.emplace_back(client_request, i + 1);
    }

    for (auto &client : clients) {
        client.join();
    }

    int expected_connections = NUM_CLIENTS;
    int actual_connections = successful_connections.load();
    EXPECT_EQ(actual_connections, expected_connections)
        << "Expected: " << expected_connections
        << ", but got: " << actual_connections;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}