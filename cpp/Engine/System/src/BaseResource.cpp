#include <Engine/System/include/BaseResource.h>
#include <Engine/System/include/GraphicsInterface.h>

void BaseResource::ParseWaitQueue() {
  while (waitQueue.empty() == false) {
    if (updateWaitQueueMutex.try_lock()) {
      if (waitQueue.front().second.lock()) {
        waitQueue.front().first();
      }
      waitQueue.pop();
      updateWaitQueueMutex.unlock();
    }
  }
  processFinished = true;
}

void BaseResource::AddToWaitQueue(std::function<void()> func,
                                  std::weak_ptr<BaseObject> obj) {
  processFinished = false;
  updateWaitQueueMutex.lock();
  bool requireThread = waitQueue.empty();
  waitQueue.push(make_pair(func, obj));
  updateWaitQueueMutex.unlock();
  if (requireThread) {
    std::thread(&BaseResource::ParseWaitQueue, this).detach();
  }
}