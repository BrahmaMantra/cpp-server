## Epoll和epoll_event
- Epoll 是一个封装了 epoll 系统调用的类，用于管理和监控多个文件描述符上的事件。（自己封装的，原本就是一个fd）
- epoll_event 是一个结构体，用于描述文件描述符上的事件。(是c++库提供的)
### 关系描述
- Epoll 类通常会包含一个 epoll 文件描述符，和一个epoll_event的数组，用来管理这些epoll_event
- epoll_event 结构体用于描述具体的事件，包括文件描述符和事件类型。（理解成一个事件）


## 遇到的坑点：
1. bzero(events, sizeof(*events) * MAX_EVENTS)和bzero(&events, sizeof(*events) * MAX_EVENTS);前者会让这个指针变成空指针，后者才是清空指定范围内存

