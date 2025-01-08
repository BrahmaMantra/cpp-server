epoll_ctl_add后会发生什么，各个参数的意义是什么

epoll_ctl 是用于控制 epoll 实例的函数。epoll_ctl 的 EPOLL_CTL_ADD 操作用于将一个新的文件描述符添加到 epoll 实例中，并指定要监视的事件。

函数原型
~~~cpp
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
~~~
参数意义
epfd：epoll 实例的文件描述符，由 epoll_create 或 epoll_create1 返回。
op：操作类型，EPOLL_CTL_ADD 表示添加一个新的文件描述符到 epoll 实例中。
fd：要添加到 epoll 实例中的文件描述符。
event：指向 epoll_event 结构体的指针，指定要监视的事件和相关数据。

question：event.data.fd和fd一定要是一样的吗，为什么？

## 水平触发和边沿触发的区别

通俗来讲，水平触发，只要有数据，epoll_wait() 就会一直返回；边沿触发，如果数据没处理完，并且这个 fd 没有新的事件，那么再次 epoll_wait() 的时候也不会有事件上来。

水平触发还是边沿触发，影响的是接收事件的行为。

（1）举个例子

假如一个 tcp socket 一次收到了 1MByte 的数据，但是 epoll_wait() 返回之后，用户一次只读取 1KB 的数据，读完之后再进行 epoll_wait() 监听事件，示例代码如下所示：
``` cpp
    for (;;) {
        event_num = epoll_wait(epoll_fd, events, 1, -1);
        ret = recv(tcp_fd, buff, 1024, 0);
        if (ret <= 0) {
            break;
       }
    }
```
如果是水平触发，那么每次 epoll_wait() 都能返回这个 tcp_fd 的事件，共返回 （1MB / 1KB）= 1024 次事件；如果是边沿触发，那么只有第一次返回了 tcp_fd 的事件，后边的 epoll_wait() 也不返回了。