set breakpoint pending on
# 设置成QEMU的路径
file ../bin/server

handle SIGUSR2 noprint nostop
handle SIGUSR1 noprint nostop

# 仅供参考，请添加任何你需要的断点。
b src/util/epoll.cpp:11
b src/util/channel.cpp:11

run 