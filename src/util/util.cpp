#include <errno.h>
#include <stdlib.h>
#include <util.h>

#include <cstring>
#include <fstream>
#include <iostream>
void errif(bool condition, const char *errmsg) {
    if (condition) {
        std::cerr << errmsg << ": " << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}
void load_env_file(const std::string &env_filename) {
    std::ifstream file(env_filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open .env file: " + env_filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        // std::cout << "line: " << line << std::endl;
        // 跳过空行或注释行
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // 简单解析 KEY=VALUE
        auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // 设置环境变量（overwrite=1表示覆盖已存在变量）
        // 设置环境变量 MY_VAR 为 "Hello, World!"
        if (setenv(key.c_str(), value.c_str(), 1) != 0) {
            perror("setenv");
            return;
        }
        // 获取并打印环境变量 MY_VAR 的值
        const char *env_value = std::getenv(key.c_str());
        if (env_value) {
            DEBUG_PRINT("%s=%s\n", key.c_str(), env_value);
        } else {
            std::cout << key << "is not set" << std::endl;
        }
    }
}