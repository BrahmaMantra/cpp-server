#include "server.h"

#include <string.h>
#include <unistd.h>

#include <functional>
#include"tcpConnection.h"

#include "acceptor.h"
#include "channel.h"
#include "socket.h"
#include<random>
#define READ_BUFFER 1024
#define SERVER_LOOP_ID_SHIFT 100
/**
 * @brief Server 构造函数，初始化服务器并设置监听
 * @param _loop 指向 EventLoop 对象的指针
 */
Server::Server(EventLoop *_loop) : main_reactor(_loop),acceptor(nullptr) {
    // 这个Acceptor对象只应该被主reactor使用
    acceptor = new Acceptor(main_reactor);
    //为了让代码可读性更好，这里显示地写出了lambda函数的信息
    std::function<void(Socket *)> acceptor_callback = [this](Socket *sock) {
        this->new_connection(sock);
    };
    acceptor->set_new_connection_callback(acceptor_callback);

    int size = std::thread::hardware_concurrency();
    thread_pool = new ThreadPool(size);
    for(int i=0;i<size;i++){
        sub_reactors.push_back(new EventLoop(SERVER_LOOP_ID_SHIFT+i));
    }
    for(int i=0;i<size;i++){
        std::function<void()> sub_loop = [this,i](){
            this->sub_reactors[i]->loop();
        };
        thread_pool->add_task(sub_loop);
    }
    thread_pool->init();
}

/**
 * @brief Server 析构函数，释放资源
 */
Server::~Server() { delete acceptor; }


/**
 * @brief 处理新的客户端连接
 * @param client_sock 指向客户端 Socket 对象的指针
 */
void Server::new_connection(Socket *client_sock) {
    if (client_sock->get_fd() == -1) {
        throw "new_connection():: socket error";
        return;
    }

    // 使用随机数种子实现负载均衡
    static std::random_device rd; // 用于生成随机种子
    static std::mt19937 gen(rd()); // 随机数生成器
    std::uniform_int_distribution<> dist(0, sub_reactors.size() - 1); // 定义范围 [0, sub_reactors.size()-1]

    int random = dist(gen); // 生成随机的子反应器索引
    TcpConnection *conn = new TcpConnection(sub_reactors[random], client_sock);

    std::function<void(Socket *)> cb = [this](Socket *_client_sock) {
        this->delete_connection(_client_sock);
    };
    conn->set_delete_callback(cb);
    connections[client_sock->get_fd()] = conn;
}

void Server::delete_connection(Socket *client_sock){
    TcpConnection *conn = connections[client_sock->get_fd()];
    connections.erase(client_sock->get_fd());
    delete conn;
}