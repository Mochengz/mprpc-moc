#include <functional>
#include <google/protobuf/descriptor.h>
#include "mprpc_provider.h"
#include "mprpc_application.h"
#include "mprpc_header.pb.h"
#include "mprpc_logger.h"
#include "zookeeper_util.h"

void MprpcProvider::NotifyService(google::protobuf::Service *service)
{

  ServiceInfo service_info;

  const google::protobuf::ServiceDescriptor *p_service_desc = service->GetDescriptor();

  // 获取服务名字
  std::string service_name = p_service_desc->name();
  // 获取服务对象service的方法数量
  int method_cnt = p_service_desc->method_count();

  LOG_INFO("service_name:%s", service_name.c_str());

  for (int i = 0; i < method_cnt; ++i)
  {
    // 获取服务对象指定下标的服务方法的描述
    const google::protobuf::MethodDescriptor *p_method_desc = p_service_desc->method(i);
    std::string method_name = p_method_desc->name();
    // 存储服务中的方法名->方法描述
    service_info.method_map.insert({method_name, p_method_desc});
    
    LOG_INFO("service_name:%s", method_name.c_str());
  }
  service_info.service_ = service;
  // 存储服务名->服务信息
  service_info_map.insert({service_name, service_info});
}

void MprpcProvider::Run()
{

  std::string ip = MprpcApplication::GetInstance().config().Load("rpc_server_ip");
  uint16_t port = stoi(MprpcApplication::GetInstance().config().Load("rpc_server_port"));
  muduo::net::InetAddress address(ip, port);

  muduo::net::TcpServer server(&event_loop_, address, "RpcProvider");

  server.setConnectionCallback(std::bind(&MprpcProvider::OnConnection, this, std::placeholders::_1));
  server.setMessageCallback(std::bind(&MprpcProvider::OnMessage, this, std::placeholders::_1,
                            std::placeholders::_2, std::placeholders::_3));

  server.setThreadNum(4);


  // 将rpc发布的服务注册到zk上
  // session timeout 30s
  // zkclient 网络io线程 1/3 * timout 发送ping消息
  ZkClient zk_cli;
  zk_cli.Start();
  // service_name 为永久性节点
  // method_name 为临时性节点
  for (auto& sp: service_info_map) {
    //   创建'/service_name'
    std::string service_path = "/" + sp.first;
    zk_cli.Create(service_path.c_str(), nullptr, 0);
    for (auto& mp : sp.second.method_map) {
      // 创建'/service_name/method_name'  存储rpc服务节点主机的ip和port
      std::string method_path = service_path + "/" + mp.first;
      char method_path_data[128] = {0};
      sprintf(method_path_data, "%s:%d", ip.c_str(), port);
      zk_cli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
    }
  }

  std::cout << "Mprpc start service at ip:" << ip << " port:" << port << std::endl;

  server.start();
  event_loop_.loop();
}

void MprpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
  if (!conn->connected())
  {
    conn->shutdown();
  }
}

void MprpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                              muduo::net::Buffer *buffer,
                              muduo::Timestamp)
{
  // 调用请求的字符流
  std::string recv_buf = buffer->retrieveAllAsString();

  uint32_t header_size = 0;
  recv_buf.copy((char *)&header_size, 4, 0);

  std::string rpc_header_str = recv_buf.substr(4, header_size);
  mprpc::RpcHeader rpc_header;
  std::string service_name;
  std::string method_name;
  uint32_t args_size;
  
  if (rpc_header.ParseFromString(rpc_header_str))
  {
    service_name = rpc_header.service_name();
    method_name = rpc_header.method_name();
    args_size = rpc_header.args_size();
  }
  else
  {
    std::cout << "rpc_header_str:" << rpc_header_str << "parse error!" << std::endl;
    return;
  }

  std::string args_str = recv_buf.substr(4 + header_size, args_size);
  // 打印调试信息
  std::cout << "============================================" << std::endl;
  std::cout << "header_size: " << header_size << std::endl;
  std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
  std::cout << "service_name: " << service_name << std::endl;
  std::cout << "method_name: " << method_name << std::endl;
  std::cout << "args_str: " << args_str << std::endl;
  std::cout << "============================================" << std::endl;

  // 获取服务与方法
  if (service_info_map.find(service_name) == service_info_map.end())
  {
    std::cout << service_name << "is not exist!" << std::endl;
    return;
  }

  auto mit = service_info_map[service_name].method_map.find(method_name);
  if (mit == service_info_map[service_name].method_map.end())
  {
    std::cout << service_name << ":" << method_name << "is not exist!" << std::endl;
    return;
  }

  google::protobuf::Service *service = service_info_map[service_name].service_; // 获取service对象 new UserService
  const google::protobuf::MethodDescriptor *method = mit->second;                   // 获取method对象 Login

  // 生成Rpc方法调用的请求和响应
  google::protobuf::Message *request = service->GetRequestPrototype(method).New();
  if (!request->ParseFromString(args_str))
  {
    std::cout << "request parse error, content:" << args_str << std::endl;
    return;
  }
  google::protobuf::Message *response = service->GetResponsePrototype(method).New();

  // 给method方法调用绑定一个closure回调函数
  google::protobuf::Closure *done = google::protobuf::NewCallback<MprpcProvider,
                                                                  const muduo::net::TcpConnectionPtr &,
                                                                  google::protobuf::Message *>(this,
                                                                                               &MprpcProvider::SendRpcResponse,
                                                                                               conn, response);
  // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
  // 回调：Closure 将数据发送给请求端
  service->CallMethod(method, nullptr, request, response, done);
}

void MprpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
  std::string response_str;
  if (response->SerializeToString(&response_str))
  {
    conn->send(response_str);
  }
  else
  {
    std::cout << "serialize response_str error" << std::endl;
  }

  conn->shutdown(); // 模拟http的短连接服务，由rpcprovider主动断开连接
}