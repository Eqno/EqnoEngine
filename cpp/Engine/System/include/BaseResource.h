#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class BaseResource {
  std::mutex updateWaitQueueMutex;
  std::queue<std::function<void()>> waitQueue;
  void ParseWaitQueue();

 public:
  void AddToWaitQueue(std::function<void()> func);
};