#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpc_application.h"
#include "mprpc_provider.h"


/*
UserService本来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendLists
*/
// 使用在rpc服务发布端（rpc提供者）
class UserService : public fixbug::UserServiceRpc{
 public:
  bool Login(std::string name, std::string pwd) {
    std::cout << "doing local service: Login" << std::endl;
    std::cout << "name:" << name << "pwd:" << pwd << std::endl;
    return true;
  }

  bool Register(uint32_t id, std::string name, std::string pwd) {
    std::cout << "doing local service: Register" << std::endl;
    std::cout << "id:" << id << "name:" << name << "pwd:" << pwd << std::endl;
    return true;
  }

  /*
  重写 基类UserServiceRpc的虚函数 下面的这些方法都是框架直接调用
  1. caller ==> Login(LoginRequest) => muduo => callee
  2. callee ==> Login(LoginRequest) => 交到下面重写的Login方法
  */
  void Login(::google::protobuf::RpcController* controller,
             const ::fixbug::LoginRequest* request,
             ::fixbug::LoginResponse* response,
             ::google::protobuf::Closure* done) {
      // 框架给业务上报了请求参数LoginRequest, 应用获取相应数据做本地业务
      std::string name = request->name();
      std::string pwd = request->pwd();

      // 做本地业务
      bool login_result = Login(name, pwd); 

      // 把响应写入
      fixbug::ResultCode *code = response->mutable_result();
      code->set_errcode(0);
      code->set_errmsg("");
      response->set_success(login_result);

      //执行回调  执行响应数据的序列化和发送
      done->Run();
  }

  void Register(::google::protobuf::RpcController* controller,
                const ::fixbug::RegisterRequest* request,
                ::fixbug::RegisterResponse* response,
                ::google::protobuf::Closure* done) {
    uint32_t id = request->id();
    std::string name = request->name();
    std::string pwd = request->pwd();

    bool ret = Register(id, name, pwd);

    response->mutable_result()->set_errcode(0);
    response->mutable_result()->set_errmsg("");
    response->set_success(ret);

    done->Run();
  }

};

int main(int argc, char** argv) {

  // 调用框架的初始化操作
  MprpcApplication::Init(argc, argv);

  // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
  MprpcProvider provider;
  provider.NotifyService(new UserService());

  // 启动rpc服务发布节点
  provider.Run();

  return 0;
}