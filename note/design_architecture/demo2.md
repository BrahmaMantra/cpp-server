目前只有Acceptor和TcpConnection会建立channel
- ![alt text](image-1.png)
- ![alt text](classes.png)

### 检测客户端是否断电等意外事故
- 用检测心跳的办法，向客户端定时发起一个hearting,设置套接字属性，设置超时值

创建的时候，先插入connections再update_channel,删除的时候先删connections在清除update_channel,插入一定要update_channel,删除一定要loop-> delete_connection(client_sock);


## 智能指针构建：
表里面用了shared_ptr的原因

valgrind检测内存泄漏