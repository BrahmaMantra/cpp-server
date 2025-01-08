#include <errno.h>
#include <stdlib.h>

#include <cstring>
#include <iostream>
void errif(bool condition, const char *errmsg) {
    if (condition) {
        std::cerr << errmsg << ": " << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}