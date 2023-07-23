#include "mprpc_config.h"
#include <iostream>
#include <string>
#include <algorithm>

void FilterStr(std::string& src_buf, char ch) {
  src_buf.erase(std::remove(src_buf.begin(), src_buf.end(), ch), src_buf.end());
}

void MprpcConfig::LoadConfigFile(const char* config_file) {
  FILE* pf = fopen(config_file, "r");
  if (pf == nullptr) {
    std::cout << config_file << " is not exist!" << std::endl;
    exit(EXIT_FAILURE);
  }

  while (!feof(pf)) {
    char buf[512] = {0};
    fgets(buf, sizeof(buf), pf);

    //去空格和\n
    std::string src_buf(buf);
    FilterStr(src_buf, ' ');
    FilterStr(src_buf, '\n');


    if (src_buf[0] == '#' || src_buf.empty()) {
      continue;
    }

    // 解析配置项
    int idx = src_buf.find('=');
    if(idx == -1) {
      //配置项不合法
      continue;
    }

    std::string key;
    std::string value;
    key = src_buf.substr(0, idx);
    value = src_buf.substr(idx + 1, src_buf.size() - idx - 1);
    config_map_.insert({key, value});
  }
}

std::string MprpcConfig::Load(const std::string& key) {
  if (config_map_.find(key) != config_map_.end()) {
    return config_map_[key];
  } else {
    return "";
  }
}

