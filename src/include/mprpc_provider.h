#pragma once
#include "google/protobuf/service.h"
#include <string>
#include <memory>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <unordered_map>

// 框架提供的专门服务发布rpc服务的网络对象类
class MprpcProvider {
 public:
 
  // 发布rpc方法的函数接口
  void NotifyService(google::protobuf::Service* service);

  // 启动rpc服务节点，开始提供rpc服务
  void Run();
 
 private:
  muduo::net::EventLoop event_loop_;

  // service服务类型信息
  // 包含服务及服务方法的信息
  struct ServiceInfo {
    google::protobuf::Service* service_;
    std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> method_map;
  };

  // 存储注册成功的服务对象名称及其服务类型信息
  std::unordered_map<std::string, ServiceInfo> service_info_map;


  //新连接回调函数
  void OnConnection(const muduo::net::TcpConnectionPtr&);

  /*
  在框架内部， RpcProvider和RpcConsumer协商好用于通信的protobuf数据类型
  service_name method_name args
  需定义proto的message类型,进行序列化和反序列化

  header_size(4 bytes) + header_str + args_str
  */
  // 已建立连接用户的读写事件回调 如果远程有rpc服务调用 那么该方法进行响应
  void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);

  // Closure的回调操作，用于序列化rpc的响应和网络发送
  void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};