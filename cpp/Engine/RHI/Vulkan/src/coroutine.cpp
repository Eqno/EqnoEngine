#include <Engine/RHI/Vulkan/include/coroutine.h>

using namespace Coroutine;

template <typename R>
TaskAwaiter<R>::TaskAwaiter(Task<R> &&task) noexcept : task(std::move(task)) {}
template <typename R>
TaskAwaiter<R>::TaskAwaiter(TaskAwaiter &&completion) noexcept
    : task(std::exchange(completion.task, {})) {}
template <typename R>
R TaskAwaiter<R>::await_resume() noexcept {
  return task.get_result();
}
template <typename R>
void TaskAwaiter<R>::await_suspend(std::coroutine_handle<> handle) noexcept {
  task.finally([handle]() { handle.resume(); });
}

Task<void> TaskPromise<void>::get_return_object() {
  return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
}