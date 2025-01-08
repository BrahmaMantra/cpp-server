cmake_minimum_required(VERSION 3.10)



# 启用debug模式
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++14")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

# 设置编译选项，启用调试符号并禁用优化
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
# 设置项目名称
project(cpp-web-server)

# 设置可执行文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 包含头文件目录
include_directories(src src/error src/util)

# 搜索 src 目录下的所有源文件
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# 搜索 src 目录下的所有源文件，排除 server.cpp 和 client.cpp
file(GLOB_RECURSE COMMON_SRC_FILES src/*.cpp)
list(REMOVE_ITEM COMMON_SRC_FILES "${CMAKE_SOURCE_DIR}/src/server.cpp")
list(REMOVE_ITEM COMMON_SRC_FILES "${CMAKE_SOURCE_DIR}/src/client.cpp")

# 添加 server 可执行文件
add_executable(server src/server.cpp ${COMMON_SRC_FILES})

# 添加 client 可执行文件
add_executable(client src/client.cpp ${COMMON_SRC_FILES})
