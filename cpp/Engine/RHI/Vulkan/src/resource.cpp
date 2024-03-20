#include <Engine/RHI/Vulkan/include/resource.h>
#include <Engine/System/include/GraphicsInterface.h>

void Resource::ParseWaitQueue() {
  if (auto graphicsPtr = graphics.lock()) {
    while (waitQueue.empty() == false) {
      if (graphicsPtr->updateMeshDataMutex.try_lock()) {
        waitQueue.front()();
        waitQueue.pop();
        graphicsPtr->updateMeshDataMutex.unlock();
      }
    }
  }
}

void Resource::AddToWaitQueue(std::function<void()> func) {
  if (auto graphicsPtr = graphics.lock()) {
    graphicsPtr->updateMeshDataMutex.lock();
    bool requireThread = waitQueue.empty();
    waitQueue.push(func);
    graphicsPtr->updateMeshDataMutex.unlock();
    if (requireThread) {
      std::thread(&Resource::ParseWaitQueue, this).detach();
    }
  }
}