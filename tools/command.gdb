# 设置成file的路径
file ../bin/server

# 仅供参考，请添加任何你需要的断点。
b src/util/epoll.cpp:11
b src/util/channel.cpp:11

run 