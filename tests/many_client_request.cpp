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
#define BUFFER_SIZE 1024

std::atomic<int> successful_connections(0);

void client_request(int thread_num) {
    try {
        Socket client_sock;
        InetAddress serv_addr(SERVER_IP, SERVER_PORT);
        client_sock.connect(&serv_addr);

        // 发送消息到服务器
        std::string message = "Hi, I'm the " + std::to_string(thread_num) + " thread";
        client_sock.write(message.c_str(), message.size());

        // 等待一秒钟，然后读取服务器的回显消息
        // sleep(1);
        char buf[BUFFER_SIZE];
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = client_sock.read(buf, sizeof(buf));
        if (read_bytes > 0) {
            std::string received_message(buf, read_bytes);

            // 去除接收到的消息中的多余换行符
            received_message.erase(received_message.find_last_not_of("\n ") + 1);

            if (received_message == message) {
                successful_connections++;
            } else {
                std::cerr << "Thread " << thread_num << ": Received unexpected message: " << received_message << std::endl;
            }
        } else if (read_bytes == 0) {
            std::cerr << "Server socket disconnected (thread " << thread_num << ")" << std::endl;
        } else if (read_bytes == -1) {
            std::cerr << "Socket read error (thread " << thread_num << ")" << std::endl;
        }

        client_sock.close();
    } catch (const SocketException &e) {
        std::cerr << "Error connecting to server (thread " << thread_num << "): " << e.what() << std::endl;
    }
}

TEST(ClientRequestsTest, MultipleConnections) {
    // std::vector<std::thread> clients;
    // for (int i = 0; i < NUM_CLIENTS; ++i) {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 延迟1毫秒
    //     clients.emplace_back(client_request, i + 1);
    // }

    // for (auto &client : clients) {
    //     client.join();
    // }

    // int expected_connections = NUM_CLIENTS;
    // int actual_connections = successful_connections.load();
    // EXPECT_EQ(actual_connections, expected_connections)
    //     << "Expected: " << expected_connections
    //     << ", but got: " << actual_connections;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}