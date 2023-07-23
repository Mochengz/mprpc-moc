#pragma once
#include <google/protobuf/service.h>
#include <string>

class MprpcController : public google::protobuf::RpcController {
 public:
  MprpcController();
  void Reset();
  bool Failed() const;
  std::string ErrorText() const;
  void SetFailed(const std::string& reason);

  void StartCancel();
  bool IsCanceled() const;
  void NotifyOnCancel(google::protobuf::Closure* callback);
 private:
  bool failed_;             // 状态信息
  std::string err_text_;    // 错误信息
};