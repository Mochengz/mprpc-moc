#pragma once
#include <queue>
#include <thread>
#include <mutex> //pthread_mutex_t
#include <condition_variable> //pthread_condition_t


//异步日志队列
template<typename T>
class MprpcLockQueue {
 public:
  // 多线程写
  void Push(const T& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(data);
    condvariable_.notify_one();
  }

  // 单线程读
  T Pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      // 日志空， 线程进入wait状态
      condvariable_.wait(lock);
    }

    T data = queue_.front();
    queue_.pop();
    return data;
  }
 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable condvariable_;
};