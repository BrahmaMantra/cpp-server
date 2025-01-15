目前只有Acceptor和TcpConnection会建立channel
- ![alt text](image-1.png)
- ![alt text](classes.png)

### 检测客户端是否断电等意外事故
- 用检测心跳的办法，向客户端定时发起一个hearting,设置套接字属性，设置超时值