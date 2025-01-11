#include <unordered_map>
#include <vector>
class EventLoop;
class Socket;
class Acceptor;
class TcpConnection;
class ThreadPool;
class Server {
   private:
    EventLoop *main_reactor;
    Acceptor *acceptor;
    std::unordered_map<int, TcpConnection *> connections;
    std::vector<EventLoop*> sub_reactors;
    ThreadPool *thread_pool;

   public:
    Server(EventLoop *loop);
    ~Server();
    // void handle_echo_event(Socket *client_sock);
    void new_connection(Socket *serv_sock);
    void delete_connection(Socket *client_sock);
};