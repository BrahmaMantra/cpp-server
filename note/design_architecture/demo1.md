## EpollLoop和Channel
- EpollLoop：里面有Epoll(目前是一个成员，我们称为ep)，提供一个update_channel函数调用ep的update_channel，来对这个channel进行更新
- Channel：是Epoll_events的封装，里面有
    - EventLoop *loop：在哪个EventLoop里面
    - int fd：文件描述符
    - uint32_t events：表示希望监听这个文件描述符的哪些事件（感兴趣的）
    - uint32_t revents：在epoll返回该Channel时文件描述符正在发生的事件（实际发生）
    - bool in_epoll
    - std::function<void()> callback:执行的回调函数
所以说，外界只知道有EventLoop和Channel，并且认为这就是Epoll的入口。在目前阶段，(服务端X)Acceptor和每个进来的客户端都会先封装一个Channel，然后通过enable_reading()才送进EpollLoop里进行检查。Server里面有一个EventLoop，用来管理这个Server的事件,然后所有封装好的Channel通过enable_reading加入到EventLoop里面，拆解成epoll_event进到EpollLoop里面
~~~ cpp
    // ep调用的update_channel函数
void Epoll::update_channel(Channel *channel) {
    int fd = channel->get_fd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->get_events();
    if (!channel->is_in_epoll()) {
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->set_in_epoll();
    } else {
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1,
              "epoll modify error");
    }
}
~~~