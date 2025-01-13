#include "server.h"

#include <string.h>
#include <unistd.h>

#include <cassert>
#include <functional>
#include <random>

#include "acceptor.h"
#include "channel.h"
#include "socket.h"
#include "tcpConnection.h"
#define READ_BUFFER 1024
#define SERVER_LOOP_ID_SHIFT 100
// Server 的处理函数,根据不同的请求类型调用不同的处理函数
void (*server_handlers[])(TcpConnection *conn){
    [HANDLE_ECHO] = hanele_echo,
    [HANDLE_HTTP] = hanele_http,
};
/**
 * @brief Server 构造函数，初始化服务器并设置监听
 * @param _loop 指向 EventLoop 对象的指针
 */
Server::Server(std::unique_ptr<EventLoop> _loop,
               std::unique_ptr<InetAddress> _addr)
    : main_reactor(std::move(_loop)) {
    // 这个Acceptor对象只应该被主reactor使用
    acceptor = std::make_unique<Acceptor>(main_reactor.get(), std::move(_addr));
    std::function<void(Socket *)> acceptor_callback = [this](Socket *sock) {
        this->new_connection(sock);
    };
    acceptor->set_new_connection_callback(acceptor_callback);

    int size = std::thread::hardware_concurrency();
    thread_pool = std::make_unique<ThreadPool>(size);
    thread_pool->init();
    // 创建子Reactor
    for (int i = 0; i < size; i++) {
        sub_reactors.push_back(
            std::make_unique<EventLoop>(SERVER_LOOP_ID_SHIFT + i));
    }
    // 把子Reactor丢进线程池
    for (int i = 0; i < size; i++) {
        std::function<void()> sub_loop = [this, i]() {
            this->sub_reactors[i]->loop();
        };
        thread_pool->add_task(std::move(sub_loop));
    }
}

/**
 * @brief Server 析构函数，释放资源
 */
Server::~Server() {}

/**
 * @brief 处理新的客户端连接
 * @param client_sock 指向客户端 Socket 对象的指针
 */
void Server::new_connection(Socket *client_sock) {
    assert(client_sock->get_fd() != -1);
    // 使用随机数种子实现简单负载均衡
    static std::random_device rd;   // 用于生成随机种子
    static std::mt19937 gen(rd());  // 随机数生成器
    std::uniform_int_distribution<> dist(
        0, sub_reactors.size() - 1);  // 定义范围 [0, sub_reactors.size()-1]

    int random = dist(gen);  // 生成随机的子反应器索引
    TcpConnection *conn =
        new TcpConnection(sub_reactors[random].get(), client_sock);

    std::function<void(Socket *)> cb = [this, conn](Socket *_client_sock) {
        this->delete_connection(_client_sock);
    };
    conn->set_close_callback(cb);
    conn->set_recv_callback(recv_callback);  // 来自Server的回调函数
    connections[client_sock->get_fd()] = conn;
}

void Server::delete_connection(Socket *client_sock) {
    auto it = connections.find(client_sock->get_fd());
    if (it == connections.end()) {
        return;
    }
    TcpConnection *conn = connections[client_sock->get_fd()];
    connections.erase(client_sock->get_fd());
    // 确认了调用delete conn就会调用TcpConnection的析构函数释放fd
    INFO_PRINT("delete_connection():client fd %d  is deleted \n", client_sock->get_fd());
    delete conn;
    conn = nullptr;
}

void Server::on_connect(std::function<void(TcpConnection *)> fn) {
    recv_callback = fn;
}

void Server::start_work() {
    // 主Reactor开始工作
    main_reactor->loop();
}

void hanele_echo(TcpConnection *conn) {
    DEBUG_PRINT("hanele_echo()\n");
    // conn->read();不用read了，写在回调里面了,再read就读不到了 QWQ
    if (conn->get_state() == TcpConnection::ConnectionState::Closed) {
        conn->handle_close();
        return;
    }
    INFO_PRINT("handle_echo():Message from client %d: %s\n",
               conn->get_socket()->get_fd(), conn->read_buffer());

    conn->set_send_buffer(conn->read_buffer());
    conn->write();
};
void hanele_http(TcpConnection *conn) {
    std::cerr << "HTTP handling not implemented yet." << std::endl;
}