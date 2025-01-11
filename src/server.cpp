#include "util/server.h"
#include "util/util.h"
#include "util/eventLoop.h"
int main() {
    load_env_file(".env");
    EventLoop *loop = new EventLoop(1);
    Server *server = new Server(loop);
    loop->loop();
    delete loop;
    delete server;
    return 0;
}