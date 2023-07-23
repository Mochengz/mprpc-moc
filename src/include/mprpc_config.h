#pragma once
#include <unordered_map>
#include <string>

// rpc_server_ip rpc_server_port zookeeper_ip zookeeper_port
// 框架读取配置文件类
class MprpcConfig {
 public:
  //负责解析加载配置文件
  void LoadConfigFile(const char* config_file);

  //查询配置项信息
  std::string Load(const std::string& key);
 private:
  std::unordered_map<std::string, std::string> config_map_;
};