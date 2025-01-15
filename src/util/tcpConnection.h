#pragma once
#include <functional>

#include "eventLoop.h"
#include "tcpConnection.h"
#include "util.h"
class Buffer;
class Socket;
class Channel;
class Server;
class EventLoop;
#define TIMER_INTERVAL 5000 //单位ms
class TcpConnection {
   public:
    enum ConnectionState {
        Invalid = 1,
        Handshaking,
        Connected,
        Closed,
        Failed,
    };
    DISALLOW_COPY_AND_MOVE(TcpConnection);
    TcpConnection(EventLoop *loop, Socket *client_sock);
    ~TcpConnection();

    void read();
    void write();
    // 当TcpConnection发起关闭请求时，进行回调，释放相应的socket.
    void handle_close();
    void send(const std::string &msg);  // 输出信息

    ConnectionState get_state();
    EventLoop *get_loop() const;

    const char *send_buffer();
    const char *read_buffer();
    Socket *get_socket();

    // 设定send buffer
    void set_send_buffer(const char *str);

    // 关闭时的回调函数
    void set_close_callback(std::function<void(Socket *)> cb);
    // 接受到信息的回调函数
    void set_recv_callback(std::function<void(TcpConnection *)> cb);

    void handle_recv();
    void init(Server *server);
   private:
    std::unique_ptr<Channel> channel;
    EventLoop *loop;
    // 该连接绑定的Socket
    Socket *client_sock;
    ConnectionState state;

    std::unique_ptr<Buffer> read_buffer_;
    std::unique_ptr<Buffer> send_buffer_;

    std::function<void(Socket *)> close_callback;
    std::function<void(TcpConnection *)> recv_callback;

    void read_nonBlocking();
    void write_nonBlocking();
    // void read_blocking();
    // void write_blocking();
    // void send_heartbeat(); // 发送心跳
};