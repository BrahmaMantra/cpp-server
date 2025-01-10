#include <unordered_map>
class EventLoop;
class Socket;
class Acceptor;
class TcpConnection;

class Server {
   private:
    EventLoop *loop;
    Acceptor *acceptor;
    std::unordered_map<int, TcpConnection *> connections;

   public:
    Server(EventLoop *loop);
    ~Server();
    // void handle_echo_event(Socket *client_sock);
    void new_connection(Socket *serv_sock);
    void delete_connection(Socket *client_sock);
};