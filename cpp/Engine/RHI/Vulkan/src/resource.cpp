#include <Engine/RHI/Vulkan/include/resource.h>
#include <Engine/System/include/GraphicsInterface.h>

void Resource::ParseWaitQueue() {
  while (waitQueue.empty() == false) {
    if (updateWaitQueueMutex.try_lock()) {
      waitQueue.front()();
      waitQueue.pop();
      updateWaitQueueMutex.unlock();
    }
  }
}

void Resource::AddToWaitQueue(std::function<void()> func) {
  updateWaitQueueMutex.lock();
  bool requireThread = waitQueue.empty();
  waitQueue.push(func);
  updateWaitQueueMutex.unlock();
  if (requireThread) {
    std::thread(&Resource::ParseWaitQueue, this).detach();
  }
}