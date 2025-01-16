#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>

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

    // 加锁保护防止多线程访问导致的数据竞争
    // 对connections封装的函数默认加锁
    std::unordered_map<int, std::weak_ptr<TcpConnection>> all_connections;

    std::vector<std::unique_ptr<EventLoop>> sub_reactors;
    std::unique_ptr<ThreadPool> thread_pool;
    std::function<void(TcpConnection *)> recv_callback;
    int handle_type;


   public:
    Server(std::unique_ptr<InetAddress> addr,int handle_type);
    ~Server();
    // void handle_echo_event(Socket *client_sock);
    void new_connection(Socket *serv_sock);
    void start_work();

    // void delete_connection(Socket *client_sock);
    // // 插入或更新连接
    // void insert_connection(int key, TcpConnection* connection);
    // // 查找连接
    // TcpConnection* find_connection(int key);


    int get_handle_type();
    void start_hearting();
};

void hanele_echo(TcpConnection *conn);
void hanele_http(TcpConnection *conn);