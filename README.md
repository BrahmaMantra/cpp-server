# cpp-server
## 前置工作
~~~ sh
sudo apt-get install libgtest-dev
~~~
## echoServer
1. cmake .
2. make all
3. ./bin/server
4. ./bin/client


## 调试
### 原生gdb调试
   1. cd tools
   2. 编写command.gdb
   3. sudo gdb -x command.gdb

### vscode的gdb调试
已经编写好了，ctrl+shift+D直接运行Debug Server，如需增加或修改任务去.vscode/launch.json和.vscode/tasks.json

...TODO!


### 常用命令
- cmake .
- make all
- make run-server
- make run-client
- ctest
   - ./bin/runTests --gtest_filter=ClientRequestsTest.*
   - ./bin/runTests --gtest_filter=ThreadPoolTest.*
- make gdb