#include <iostream>
#include "mprpc_application.h"
#include "friend.pb.h"

int main(int argc, char** argv) {
  // mprpc调用端

  MprpcApplication::Init(argc, argv);

  fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());
  
  // 测试Login
  fixbug::GetFriendsListRequest request;
  request.set_user_id(1000);

  fixbug::GetFriendsListResponse response;

  MprpcController  controller;
  stub.GetFriendsList(&controller, &request, &response, nullptr); // 统一调用RpcChannel->callMethod 进行序列化和网络发送
  if (controller.Failed()) {
    std::cout << controller.ErrorText() << std::endl;
  } else {
    if (response.result().errcode() == 0) {
      std::cout << "rpc GetFriendList response:" << response.success() << std::endl;
      int size = response.friends_size();
      for (int i = 0; i < size; ++i) {
        std::cout << "index:" << (i + 1) << "name:" << response.friends(i) << std::endl;
      }
    } else {
      std::cout << "rpc GetFriendList response error:" << response.result().errmsg() << std::endl;
    }
  }
  return 0;
}