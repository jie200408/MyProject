// #include "../../MqCommon/logger.hpp"
#include <iostream>
#include <ctime>

#define DEBUG_LEVEL 0
#define INFO_LEVEL 1
#define ERROR_LEVEL 2
#define DEFAULT_LEVEL DEBUG_LEVEL

#define LOG(log_level_str, log_level, format, ...) do {\
    if (log_level >= DEFAULT_LEVEL) { \
        time_t t = time(nullptr);\
        struct tm* ptm = localtime(&t);\
        char timestr[32];\
        strftime(timestr, 31, "%H:%M:%S", ptm);\
        printf("[%s][%s][%s:%d] " format "", log_level_str, timestr, __FILE__, __LINE__, ##__VA_ARGS__);\
    }                                                        \
} while(0)

#define DLOG(format, ...) LOG("DEBUG", DEBUG_LEVEL, format, ##__VA_ARGS__)
#define ILOG(format, ...) LOG("INFO", DEBUG_LEVEL, format, ##__VA_ARGS__)
#define ELOG(format, ...) LOG("ERROR", DEBUG_LEVEL, format, ##__VA_ARGS__)

int main() {
    time_t t = time(nullptr);
    struct tm* ptm = localtime(&t);
    char timestr[32];
    strftime(timestr, 31, "%H:%M:%S", ptm);
    // printf("[%s][%s:%d] helloworld\n", timestr, __FILE__, __LINE__);
    // LOG(DEBUG_LEVEL, "hello %d\n", 2);
    DLOG("xxxx%d\n",2);
    ELOG("hell %s\n", "dkad");

    return 0;
}