#include "socket_exception.h"


SocketException::SocketException(SocketError error) : error_(error), msg_(getErrorMessage(error)) {}

const char* SocketException::what() const noexcept {
    return msg_.c_str();
}

string SocketException::getErrorMessage(SocketError error) const {
    switch (error) {
        case SocketError::CONNECTION_FAILED:
            return "Connection failed";
        case SocketError::TIMEOUT:
            return "Timeout occurred";
        case SocketError::UNKNOWN_HOST:
            return "Unknown host";
        case SocketError::NETWORK_UNREACHABLE:
            return "Network unreachable";
        case SocketError::BIND_FAILED:
            return "Bind failed";
        case SocketError::LISTEN_FAILED:
            return "Listen failed";
        case SocketError::ACCEPT_FAILED:
            return "Accept failed";
        case SocketError::SEND_FAILED:
            return "Send failed";
        case SocketError::RECEIVE_FAILED:
            return "Receive failed";
        default:
            return "Unknown socket error";
    }
}
// 直接抛出异常
void handleError(SocketError error) {
    throw SocketException(error);
}