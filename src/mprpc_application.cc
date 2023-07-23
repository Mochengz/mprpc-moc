#include "mprpc_application.h"
#include <iostream>
#include <unistd.h>

MprpcConfig MprpcApplication::config_; 

void ShowArgsHelp() {
  std::cout << "format: command -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char** argv) {
  if (argc < 2) {
    ShowArgsHelp();
    exit(EXIT_FAILURE);
  }

  int c = 0;
  std::string config_file;
  while ((c = getopt(argc, argv, "i:")) != -1) {
    switch (c) {
      case 'i':
        config_file = optarg;
        break;
      case '?':
        std::cout << "invalid args!" << std::endl;
        ShowArgsHelp();
        exit(EXIT_FAILURE);
      case ':':
        std::cout << "need <configfile>" << std::endl;
        ShowArgsHelp();
        exit(EXIT_FAILURE);
      default:
        break;
    }
  }

  //开始加载配置文件
  config_.LoadConfigFile(config_file.c_str());

  //std::cout <<"rpcserverip:" << m_config.Load("rpc_server_ip") << std::endl;
  //std::cout <<"rpcserverport:" << m_config.Load("rpc_server_port") << std::endl;
  //std::cout <<"zookeeperip:" << m_config.Load("zookeeper_ip") << std::endl;
  //std::cout <<"zookeeperport" << m_config.Load("zookeeper_port") << std::endl;
}


MprpcApplication& MprpcApplication::GetInstance() {
  static MprpcApplication app;
  return app;
}

MprpcConfig& MprpcApplication::config() {
  return config_;
}