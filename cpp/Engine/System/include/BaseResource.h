#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class GraphicsInterface;
class BaseResource {
  std::weak_ptr<GraphicsInterface> graphics;

  std::mutex updateWaitQueueMutex;
  std::queue<std::function<void()>> waitQueue;
  void ParseWaitQueue();

 public:
  void SetGraphics(std::weak_ptr<GraphicsInterface> graphics) {
    this->graphics = graphics;
  }
  void AddToWaitQueue(std::function<void()> func);
};