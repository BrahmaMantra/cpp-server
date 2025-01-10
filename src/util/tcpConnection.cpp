#include "tcpConnection.h"

#include <string.h>

#include "channel.h"
#include "socket.h"
#define READ_BUFFER 1024

TcpConnection::TcpConnection(EventLoop *loop, Socket *_client_sock)
    : loop(loop), client_sock(_client_sock) {
    channel = new Channel(loop, client_sock->get_fd());
    std::function<void()> channel_callback = [this]() {
        this->handle_echo_server(client_sock);
    };
    channel->set_read_callback(channel_callback);
    channel->enable_reading();
}
TcpConnection::~TcpConnection() {
    delete channel;
    delete client_sock;
}

void TcpConnection::handle_echo_server(Socket *_client_sock){
    char buf[READ_BUFFER];
    while(1){
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = _client_sock->read(buf, sizeof(buf));
                if (read_bytes > 0) {
            std::cout << "message from client fd " << _client_sock->get_fd()
                      << ": " << buf << std::endl;
            _client_sock->write(buf, read_bytes);  // echo服务
        } else if (read_bytes == 0) {
            std::cout << "client fd " << _client_sock->get_fd()
                      << " is disconnected" << std::endl;
            _client_sock->close();
            break;
        } else if (read_bytes == -1 &&
                   errno == EINTR) {  //客户端正常中断、继续读取
            printf("continue reading");
            continue;
        } else if (read_bytes == -1 &&
                   ((errno == EAGAIN) ||
                    (errno ==
                     EWOULDBLOCK))) {  //非阻塞IO，这个条件表示数据全部读取完毕
            // printf("finish reading once, errno: %d\n", errno);
            break;
        }

    }
}

void TcpConnection::set_delete_callback(std::function<void(Socket *)> cb){
    delete_callback = cb;
}