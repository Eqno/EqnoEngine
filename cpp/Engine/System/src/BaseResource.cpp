#include <Engine/System/include/BaseResource.h>
#include <Engine/System/include/GraphicsInterface.h>

void BaseResource::ParseWaitQueue() {
  while (waitQueue.empty() == false) {
    if (updateWaitQueueMutex.try_lock()) {
      waitQueue.front().first();
      waitQueue.pop();
      updateWaitQueueMutex.unlock();
    }
  }
}

void BaseResource::AddToWaitQueue(std::function<void()> func,
                                  std::shared_ptr<BaseObject> obj) {
  updateWaitQueueMutex.lock();
  bool requireThread = waitQueue.empty();
  waitQueue.push(make_pair(func, obj));
  updateWaitQueueMutex.unlock();
  if (requireThread) {
    std::thread(&BaseResource::ParseWaitQueue, this).detach();
  }
}