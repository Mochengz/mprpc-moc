# C++ RPC framework based on muduo and protobuf



## Introduction  

本项目为基于C++编写的分布式RPC通信框架。使用muduo网络库实现高并发的RPC同步调用请求处理，使用Protobuf作为RPC方法调用和参数的序列化。

---



## Environment

* OS: Ubuntu 20.04.1
* Complier: g++ 9.4.0
* Automated build tool: CMake 3.16.3
* Data-Serialization tool: Protobuf 3.11.0
* Centralized Service tool: ZooKeeper 3.4.10

## Build

	./build.sh

## Usage

	cd ./bin

1. 开启服务请求方

   ```
   ./consumer
   ```

2. 开启服务提供方

   ```
   ./provider
   ```

## Technical points

* 使用muduo 网络库实现高并发的RPC 同步调用请求处理
* 使用Protobuf 作为RPC 方法调用和参数的序列化和反序列化
* 基于线程安全的缓冲队列实现异步日志输出
* 使用zookeeper 作为服务治理中间件，提供服务注册和服务发现功能

## 代码统计

![image](https://github.com/Mochengz/mprpc-moc/blob/master/images/code_sum.png)

## 文件树

![image](https://github.com/Mochengz/mprpc-moc/blob/master/images/tree.png)



## Model

