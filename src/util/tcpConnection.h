#pragma once
#include<functional>

class EventLoop;
class Socket;
class Channel;
class TcpConnection{
    private:
    EventLoop *loop;
    Socket *client_sock;
    Channel *channel;
    std::function<void(Socket *)> delete_callback;

    public:
    TcpConnection(EventLoop *_loop, Socket *_sock);
    ~TcpConnection();

    void handle_echo_server(Socket *client_sock);
    void set_delete_callback(std::function<void(Socket *)> cb);
};