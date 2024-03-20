#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class GraphicsInterface;

class Resource {
  std::weak_ptr<GraphicsInterface> graphics;
  std::queue<std::function<void()>> waitQueue;
  void ParseWaitQueue();

 public:
  void SetGraphicsInterface(std::weak_ptr<GraphicsInterface> graphics) {
    this->graphics = graphics;
  }
  void AddToWaitQueue(std::function<void()> func);
};