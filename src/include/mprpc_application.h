#pragma once
#include "mprpc_config.h"
#include "mprpc_channel.h"
#include "mprpc_controller.h"

// mprpc框架的基础类,提供初始化
class MprpcApplication {
 public:
  static void Init(int argc, char** argv);
  static MprpcApplication& GetInstance();
  static MprpcConfig& config();

 private:
  static MprpcConfig config_;
  static MprpcApplication mprpc_application_;

  MprpcApplication() {}
  MprpcApplication(const MprpcApplication&) = delete;
  MprpcApplication(MprpcApplication&) = delete;
};