#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

#include "error/socket_exception.h"

#define NUM_CLIENTS 20000
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7777
#define BUFFER_SIZE 1024
#define HEARTBEAT_MSG "HEARTBEAT"
#define HEARTBEAT_RESPONSE "ALIVE"
// 线程创立间隔
#define SLEEP_MICROSECONDS 10

std::atomic<int> successful_connections(0);

void client_request(int thread_num) {
    try {
        int client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock < 0) {
            std::cerr << "Failed to create socket (thread " << thread_num << ")" << std::endl;
            return;
        }

        sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SERVER_PORT);
        if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address/ Address not supported (thread " << thread_num << ")" << std::endl;
            close(client_sock);
            return;
        }

        if (connect(client_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Connection failed (thread " << thread_num << ")" << std::endl;
            close(client_sock);
            return;
        }

        std::string message = "Hi, I'm the " + std::to_string(thread_num) + " thread";
        ssize_t sent_bytes = send(client_sock, message.c_str(), message.size(), 0);

        if (sent_bytes < 0) {
            std::cerr << "Send failed (thread " << thread_num << ")" << std::endl;
            close(client_sock);
            return;
        }

        char buf[BUFFER_SIZE];
        memset(buf, 0, BUFFER_SIZE);

        // std::this_thread::sleep_for(std::chrono::microseconds(SLEEP_MICROSECONDS));
        while (true) {
            ssize_t read_bytes = recv(client_sock, buf, BUFFER_SIZE, 0);
            if (read_bytes > 0) {
                std::string received_message(buf, read_bytes);
                received_message.erase(received_message.find_last_not_of("\n ") + 1);
                if (received_message == message) {
                    successful_connections++;
                    break;
                } else if (received_message == HEARTBEAT_MSG) {
                    ssize_t sent_bytes = send(client_sock, HEARTBEAT_RESPONSE, strlen(HEARTBEAT_RESPONSE), 0);
                    if (sent_bytes < 0) {
                        std::cerr << "Failed to send heartbeat response (thread " << thread_num << ")" << std::endl;
                        close(client_sock);
                        return;
                    }
                } else {
                    std::cerr << "Thread " << thread_num << ": Received unexpected message: " << received_message << std::endl;
                }
            } else if (read_bytes == 0) {
                std::cerr << "Server socket disconnected (thread " << thread_num << ")" << std::endl;
                break;
            } else {
                std::cerr << "Socket read error (thread " << thread_num << ")" << std::endl;
                break;
            }
        }

        close(client_sock);
        std::cout << "Thread " << thread_num << " finished" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Exception in thread " << thread_num << ": " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in thread " << thread_num << std::endl;
    }
}

TEST(ClientRequestsTest, MultipleConnections) {
    try {
        std::vector<std::thread> clients;
        for (int i = 0; i < NUM_CLIENTS; ++i) {
            std::this_thread::sleep_for(std::chrono::microseconds(SLEEP_MICROSECONDS));
            clients.emplace_back(client_request, i + 1);
        }

        for (auto &client : clients) {
            client.join();
        }

        int expected_connections = NUM_CLIENTS;
        int actual_connections = successful_connections.load();
        EXPECT_EQ(actual_connections, expected_connections)
            << "Expected: " << expected_connections << ", but got: " << actual_connections;
    } catch (const std::exception &e) {
        std::cerr << "Exception in test: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in test" << std::endl;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}