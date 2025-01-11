#pragma once
#include <string>
void errif(bool, const char *);
void load_env_file(const std::string &env_filename) ;
void debugPrint(const std::string &message);