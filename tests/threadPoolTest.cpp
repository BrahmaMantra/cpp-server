// #include <gtest/gtest.h>
// #include <iostream>
// #include <fstream>
// #include <string>
// #include <functional>
// #include "threadPool/threadPool.h"

// // 打开输出文件
// std::ofstream output_file("test_output.txt");

// // 打印函数
// void print(int a, double b, const char *c, std::string d) {
//     output_file << a << " " << b << " " << c << " " << d << std::endl;
// }

// // 测试函数
// void test() { output_file << "hellp" << std::endl; }

// // 测试线程池的基本功能
// TEST(ThreadPoolTest, BasicFunctionality) {
//     // 创建一个线程池
//     ThreadPool pool(4);
//     pool.init();

//     // 提交任务
//     std::function<void()> func = std::bind(print, 1, 3.14, "hello", std::string("world"));
//     pool.submit(func);
//     func = test;
//     pool.submit(func);

//     // 关闭线程池
//     pool.shutdown();
// }