#include <string>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <error.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "mprpc_header.pb.h"
#include "mprpc_application.h"
#include "zookeeper_util.h"

/*
header_size + service_name method_name args_size + args
*/    
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                google::protobuf::RpcController* controller, 
                const google::protobuf::Message* request,
                google::protobuf::Message* response, google::protobuf::Closure* done) {

    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str)) {
      args_size = args_str.size();
    } else {
      controller->SetFailed("serialize request error!");
      return;
    }

    //定义rpc的请求header
    mprpc::RpcHeader rpc_header;
    rpc_header.set_service_name(service_name);
    rpc_header.set_method_name(method_name);
    rpc_header.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpc_header.SerializeToString(&rpc_header_str)) {
      header_size = rpc_header_str.size();
    } else {
      controller->SetFailed("serialize rpc header error!");
      return;
    }

    // 组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl; 
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name: " << method_name << std::endl; 
    std::cout << "args_str: " << args_str << std::endl; 
    std::cout << "============================================" << std::endl;

    //tcp完成Rpc远程调用
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
      char errtext[512] = {0};
      sprintf(errtext, "create socket error!%d", errno);
      controller->SetFailed(errtext);
      return;
    }

    //std::string ip = MprpcApplication::GetInstance().config().Load("rpc_server_ip");
    //uint16_t port = stoi(MprpcApplication::GetInstance().config().Load("rpc_server_port"));
    ZkClient zk_cli;
    zk_cli.Start();
    std::string method_path = '/' + service_name + "/" + method_name;
    std::string host_data = zk_cli.GetData(method_path.c_str());
    if (host_data == "") {
      controller->SetFailed(method_path + " is not exist!");
      return;
    }
    int idx = host_data.find(":");
    if (idx == - 1) {
      controller->SetFailed(method_path + " address is invalid!");
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = stoi(host_data.substr(idx + 1, host_data.size() - idx));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
      close(client_fd);
      char errtext[512] = {0};
      sprintf(errtext, "connect error! errno:%d", errno);
      controller->SetFailed(errtext);
      return;
    }

    if (send(client_fd, send_rpc_str.c_str(), send_rpc_str.size(), 0) == -1) {
      close(client_fd);
      char errtext[512] = {0};
      sprintf(errtext, "send error! errno:%d", errno);
      controller->SetFailed(errtext);
      return;
    }

    char recv_buf[1024] = {0};
    int recv_size = 0;
    if ((recv_size = recv(client_fd, recv_buf, sizeof(recv_buf), 0)) == -1)  {
      close(client_fd);
      char errtext[512] = {0};
      sprintf(errtext, "recv error! errno:%d", errno);
      controller->SetFailed(errtext);
      return;
    }

    //std::string response_str(recv_buf, 0, recv_size);
    //rpc返回结果序列化
    if (!response->ParseFromArray(recv_buf, recv_size)) {
      close(client_fd);
      char errtext[2056] = {0};
      sprintf(errtext, "parse error! response_str:%s", recv_buf);
      controller->SetFailed(errtext);
      return;
    }
    close(client_fd);

}
