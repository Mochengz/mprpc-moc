#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpc_application.h"
#include "mprpc_provider.h"
#include <vector>

class FriendService : public fixbug::FriendServiceRpc {
 public:
  std::vector<std::string> GetFriendsList(uint32_t user_id) {
    std::cout << "do GetFriendsList service" << user_id << std::endl;
    std::vector<std::string> vec;
    vec.push_back("gaoy");
    vec.push_back("liuh");
    vec.push_back("wangs");

    return vec;
  }

  void GetFriendsList(::google::protobuf::RpcController* controller,
                      const ::fixbug::GetFriendsListRequest* request,
                      ::fixbug::GetFriendsListResponse* response,
                      ::google::protobuf::Closure* done) {
    uint32_t user_id = request->user_id();

    std::vector<std::string> friends_list = GetFriendsList(user_id);
    response->mutable_result()->set_errcode(0);
    response->mutable_result()->set_errmsg("");
    for (auto& name : friends_list) {
      std::string* p = response->add_friends();
      *p = name;
    }
    done->Run();
  }
 private:
};

int main(int argc, char** argv) {

  // 调用框架的初始化操作
  MprpcApplication::Init(argc, argv);

  // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
  MprpcProvider provider;
  provider.NotifyService(new FriendService());

  // 启动rpc服务发布节点
  provider.Run();

  return 0;
}