#include "mprpc_logger.h"
#include <time.h>
#include <iostream>



MprpcLogger& MprpcLogger::GetInstance() {
  static MprpcLogger logger;
  return logger;
}

MprpcLogger::MprpcLogger() {
  // 启动专门写日志线程
  std::thread writeLogTask([&](){
    for (;;) {
      // 获取当前日期,追加写入文件
      time_t now = time(nullptr);
      tm* nowtm = localtime(&now);

      char file_name[128];
      sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);

      FILE *pf = fopen(file_name, "a+");
      if (pf == nullptr) {
        std::cout << "logger file :" << file_name << "open error!" << std::endl;
        exit(EXIT_FAILURE);
      }
      
      std::string msg = lock_queue_.Pop();
      char time_buf[128] = {0};
      sprintf(time_buf, "%d:%d:%d =>[%s] ", 
              nowtm->tm_hour, 
              nowtm->tm_min, 
              nowtm->tm_sec,
              (log_level_ == INFO ? "info" : "error"));
            msg.insert(0, time_buf);
            msg.append("\n");
      fputs(msg.c_str(), pf);
      fclose(pf);
    }
  });

  writeLogTask.detach();
}


void MprpcLogger::set_log_level(LogLevel level) {
  log_level_ = level;
}

void MprpcLogger::Log(std::string msg) {
  lock_queue_.Push(msg);
}