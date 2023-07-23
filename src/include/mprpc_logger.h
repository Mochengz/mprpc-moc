#pragma once
#include "mprpc_lockqueue.h"

//日志信息级别
enum LogLevel {
  INFO, //普通信息
  ERROR, //错误信息
};

// 日志系统
class MprpcLogger {
 public:
  static MprpcLogger& GetInstance();
  void set_log_level(LogLevel level);
  void Log(std::string msg);
 private:
  int log_level_;  //日志级别
  MprpcLockQueue<std::string> lock_queue_; //日志缓冲队列

  MprpcLogger();
  MprpcLogger(const MprpcLogger&) = delete;
  MprpcLogger(MprpcLogger&&) = delete;
};


// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        MprpcLogger &logger = MprpcLogger::GetInstance(); \
        logger.set_log_level(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) 

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        MprpcLogger &logger = MprpcLogger::GetInstance(); \
        logger.set_log_level(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) 
