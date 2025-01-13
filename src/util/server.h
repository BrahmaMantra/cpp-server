#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

class EventLoop;
class Socket;
class Acceptor;
class TcpConnection;
class ThreadPool;
class InetAddress;

#define HANDLE_HTTP 1
#define HANDLE_ECHO 0
// Server 的处理函数,根据不同的请求类型调用不同的处理函数
extern void (*server_handlers[])(TcpConnection *conn);

class Server {
   private:
    std::unique_ptr<EventLoop> main_reactor;
    std::unique_ptr<Acceptor> acceptor;
    std::unordered_map<int, TcpConnection *> connections;
    std::vector<std::unique_ptr<EventLoop>> sub_reactors;
    std::unique_ptr<ThreadPool> thread_pool;
    std::function<void(TcpConnection *)> recv_callback;

   public:
    Server(std::unique_ptr<EventLoop> loop, std::unique_ptr<InetAddress> addr);
    ~Server();
    // void handle_echo_event(Socket *client_sock);
    void new_connection(Socket *serv_sock);
    void delete_connection(Socket *client_sock);
    void on_connect(std::function<void(TcpConnection *)> fn);
    void start_work();
};

void hanele_echo(TcpConnection *conn);
void hanele_http(TcpConnection *conn);