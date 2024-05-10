#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class BaseObject;
class GraphicsInterface;

class BaseResource {
  std::mutex updateWaitQueueMutex;
  std::queue<std::pair<std::function<void()>, std::weak_ptr<BaseObject>>>
      waitQueue;
  void ParseWaitQueue();

 public:
  void AddToWaitQueue(std::function<void()> func,
                      std::weak_ptr<BaseObject> obj);
};