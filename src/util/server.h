
class EventLoop;
class Socket;
class Server {
   private:
    EventLoop *loop;
    Socket *serv_sock;

   public:
    Server(EventLoop *loop);
    ~Server();
    void handle_echo_event(Socket *client_sock);
    void new_connection(Socket *serv_sock);
};