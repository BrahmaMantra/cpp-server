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