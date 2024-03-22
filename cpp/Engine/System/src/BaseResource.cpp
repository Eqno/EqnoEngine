#include <Engine/System/include/BaseResource.h>
#include <Engine/System/include/GraphicsInterface.h>

void BaseResource::ParseWaitQueue() {
  while (waitQueue.empty() == false) {
    if (updateWaitQueueMutex.try_lock()) {
      waitQueue.front()();
      waitQueue.pop();
      updateWaitQueueMutex.unlock();
    }
  }
}

void BaseResource::AddToWaitQueue(std::function<void()> func) {
  updateWaitQueueMutex.lock();
  bool requireThread = waitQueue.empty();
  waitQueue.push(func);
  updateWaitQueueMutex.unlock();
  if (requireThread) {
    std::thread(&BaseResource::ParseWaitQueue, this).detach();
  }
}