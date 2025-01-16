#pragma once
#include <cstdlib>
#include <iostream>
#include <string>

void errif(bool, const char *);
void load_env_file(const std::string &env_filename);
#define HEARTBEAT_MSG "HEARTBEAT"
#define HEARTBEAT_RESPONSE "ALIVE"


// Macros to disable copying and moving
#define DISALLOW_COPY(cname)       \
    cname(const cname &) = delete; \
    cname &operator=(const cname &) = delete;

#define DISALLOW_MOVE(cname)  \
    cname(cname &&) = delete; \
    cname &operator=(cname &&) = delete;

#define DISALLOW_COPY_AND_MOVE(cname) \
    DISALLOW_COPY(cname);             \
    DISALLOW_MOVE(cname);

#define DEBUG_PRINT(format, ...)                      \
    do {                                              \
        const char *debugEnv = std::getenv("DEBUG");  \
        if (debugEnv && std::string(debugEnv) == "1") \
            printf(format, ##__VA_ARGS__);            \
    } while (0)

#define INFO_PRINT(format, ...)                       \
    do {                                              \
        const char *debugEnv = std::getenv("INFO");   \
        if (debugEnv && std::string(debugEnv) == "1") \
            printf(format, ##__VA_ARGS__);            \
    } while (0)
