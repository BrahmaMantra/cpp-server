#include "server.h"

#include <string.h>
#include <unistd.h>

#include <functional>
#include"tcpConnection.h"
#include "InetAddress.h"
#include "acceptor.h"
#include "channel.h"
#include "socket.h"

#define READ_BUFFER 1024

/**
 * @brief Server 构造函数，初始化服务器并设置监听
 * @param _loop 指向 EventLoop 对象的指针
 */
Server::Server(EventLoop *_loop) : loop(_loop) {
    // 把bind listen，accept等细节封装到Acceptor中
    acceptor = new Acceptor(loop);
    
    //为了让代码可读性更好，这里显示地写出了lambda函数的信息
    std::function<void(Socket *)> acceptor_callback = [this](Socket *sock) {
        this->new_connection(sock);
    };
    acceptor->set_new_connection_callback(acceptor_callback);
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
    TcpConnection *conn = new TcpConnection(loop, client_sock);
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