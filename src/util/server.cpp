#include "server.h"

#include <string.h>
#include <unistd.h>

#include <functional>

#include "InetAddress.h"
#include "channel.h"
#include "socket.h"

#define READ_BUFFER 1024

/**
 * @brief Server 构造函数，初始化服务器并设置监听
 * @param _loop 指向 EventLoop 对象的指针
 */
Server::Server(EventLoop *_loop) : loop(_loop) {
    Socket *serv_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 7777);
    serv_sock->bind(serv_addr);
    serv_sock->listen();
    serv_sock->setNonblocking();

    Channel *servChannel = new Channel(loop, serv_sock->get_fd());
    std::function<void()> callback =
        std::bind(&Server::new_connection, this, serv_sock);
    servChannel->set_callback(callback);
    //这一步会把Channel加入Loop的epoll中
    servChannel->enable_reading();
}

/**
 * @brief Server 析构函数，释放资源
 */
Server::~Server() {}

/**
 * @brief 处理客户端的 echo 事件
 * @param client_sock 指向客户端 Socket 对象的指针
 */
void Server::handle_echo_event(Socket *client_sock) {
    char buf[READ_BUFFER];
    std::cout << "client fd is " << client_sock->get_fd() << std::endl;
    while (true) {
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = client_sock->read(buf, sizeof(buf));
        if (read_bytes > 0) {
            std::cout << "message from client fd " << client_sock->get_fd()
                      << ": " << buf << std::endl;
            client_sock->write(buf, sizeof(buf));  // echo服务
        } else if (read_bytes == 0) {
            std::cout << "client fd " << client_sock->get_fd()
                      << " is disconnected" << std::endl;
            client_sock->close();
            break;
        } else if (read_bytes == -1 &&
                   errno == EINTR) {  //客户端正常中断、继续读取
            printf("continue reading");
            continue;
        } else if (read_bytes == -1 &&
                   ((errno == EAGAIN) ||
                    (errno ==
                     EWOULDBLOCK))) {  //非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            break;
        }
    }
}

/**
 * @brief 处理新的客户端连接
 * @param serv_sock 指向服务器 Socket 对象的指针
 */
void Server::new_connection(Socket *serv_sock) {
    InetAddress *clnt_addr = new InetAddress();  //会发生内存泄露！没有delete
    Socket *clnt_sock =
        new Socket(serv_sock->accept(clnt_addr));  //会发生内存泄露！没有delete
    clnt_sock->setNonblocking();
    std::cout << "new client connected, fd is " << clnt_sock->get_fd()
              << std::endl;
    Channel *clntChannel = new Channel(loop, clnt_sock->get_fd());
    // 第一个参数是成员函数指针，第二个参数隐式绑定this指针，第三个参数是函数参数
    std::function<void()> callback =
        std::bind(&Server::handle_echo_event, this, clnt_sock);
    clntChannel->set_callback(callback);
    //这一步会把Channel加入Loop的epoll中
    clntChannel->enable_reading();
}