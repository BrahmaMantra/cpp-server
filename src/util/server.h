
class EventLoop;
class Socket;
class Acceptor;
class Server {
   private:
    EventLoop *loop;
    Acceptor *acceptor;

   public:
    Server(EventLoop *loop);
    ~Server();
    void handle_echo_event(Socket *client_sock);
    void new_connection(Socket *serv_sock);
};