#include<exception>
#include<string.h>
#include<iostream>
using namespace std;
// 枚举常见的socket错误
enum class SocketError {
    CONNECTION_FAILED,
    TIMEOUT,
    UNKNOWN_HOST,
    NETWORK_UNREACHABLE,
    BIND_FAILED,
    LISTEN_FAILED,
    ACCEPT_FAILED,
    SEND_FAILED,
    RECEIVE_FAILED
};

//自定义异常类
class SocketException :public std::exception{
public:
    // 禁止隐式构造
    explicit SocketException(SocketError error);

    virtual const char* what() const noexcept override;

private:
    string getErrorMessage(SocketError error) const;

    SocketError error_;
    string msg_;
};
// 直接抛出异常
void handleError(SocketError error);