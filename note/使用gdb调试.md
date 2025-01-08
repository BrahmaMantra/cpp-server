# 使用gdb调试

## 准备前提：
1. 在CMakeLists顶部添加下行,确保程序是可debug的
~~~ sh
# 启用debug模式
set(CMAKE_BUILD_TYPE Debug)
# 设置编译选项，启用调试符号并禁用优化
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
~~~
1. 编写.gdbinit,按照提示把此.gdbinit加到gdbinit的配置里面,然后编写一个command.gdb
    - 遇到的问题：watch的时候
~~~ sh
Epoll::Epoll (this=0x5555555702f0) at /home/fz/cpp-project/cpp-server/src/util/epoll.cpp:13
13          std::cout<<"Epoll(): epfd is "<<epfd<<",events is "<<events<<std::endl;
(gdb) n
current stack frame does not contain a variable named `this'
~~~
    - 解决方案：使用watch *(epoll_event *) 0x5555555702f8直接观测内存


## 为什么不用vscode集成的gdb调试器？
因为用不惯...
