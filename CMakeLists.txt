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

# 添加自定义目标 run-server
add_custom_target(run-server
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server
    DEPENDS server
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Running server..."
)

# 添加自定义目标 run-client
add_custom_target(run-client
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client
    DEPENDS client
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Running client..."
)


# 启用测试
enable_testing()

# 添加测试源文件
file(GLOB TEST_SRC_FILES tests/*.cpp)

# 添加测试可执行文件
add_executable(runTests ${TEST_SRC_FILES} ${COMMON_SRC_FILES})

# 链接测试可执行文件所需的库
target_link_libraries(runTests gtest gtest_main pthread)

# 添加测试
add_test(NAME runTests COMMAND runTests)

# 添加自定义目标 gdb
add_custom_target(gdb
    COMMAND cd tools && gdb -x command.gdb
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Running gdb with command.gdb..."
)

# 添加自定义目标 gdb
add_custom_target(fmt
    COMMAND cd tools && bash format.sh
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Making fmt..."
)