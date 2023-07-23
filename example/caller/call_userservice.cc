#include <iostream>
#include "mprpc_application.h"
#include "user.pb.h"
#include "mprpc_channel.h"

int main(int argc, char** argv) {
  // mprpc调用端

  MprpcApplication::Init(argc, argv);

  fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
  
  // 测试Login
  fixbug::LoginRequest request;
  request.set_name("zhang san");
  request.set_pwd("123456");

  fixbug::LoginResponse response;
  
  stub.Login(nullptr, &request, &response, nullptr); // 统一调用RpcChannel->callMethod 进行序列化和网络发送

  if (response.result().errcode() == 0) {
    std::cout << "rpc login response:" << response.success() << std::endl;
  } else {
    std::cout << "rpc login response error:" << response.result().errmsg() << std::endl;
  }

  // 测试Register
  fixbug::RegisterRequest req;
  req.set_id(2000);
  req.set_name("mprpc");
  req.set_pwd("6666");
  fixbug::RegisterResponse rsp;

  stub.Register(nullptr, &req, &rsp, nullptr);

  if (rsp.result().errcode() == 0) {
    std::cout << "rpc register response success:" << rsp.success() << std::endl;
  } else {
    std::cout << "rpc register response error:" << rsp.result().errmsg() << std::endl;
  }

  return 0;
}