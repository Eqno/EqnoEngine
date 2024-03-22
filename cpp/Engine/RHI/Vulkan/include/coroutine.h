#pragma once

#include <chrono>
#include <coroutine>
#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

namespace Coroutine {
template <typename ResultType>
struct Task;

template <typename T>
struct Result {
  explicit Result() = default;
  explicit Result(T &&value) : _value(value) {}
  explicit Result(std::exception_ptr &&exception_ptr)
      : _exception_ptr(exception_ptr) {}
  T get_or_throw() {
    if (_exception_ptr) {
      std::rethrow_exception(_exception_ptr);
    }
    return _value;
  }
  T _value{};
  std::exception_ptr _exception_ptr;
};
template <>
struct Result<void> {
  explicit Result() = default;
  explicit Result(std::exception_ptr &&exception_ptr)
      : _exception_ptr(exception_ptr) {}
  void get_or_throw() {
    if (_exception_ptr) {
      std::rethrow_exception(_exception_ptr);
    }
  }
  std::exception_ptr _exception_ptr;
};
template <typename R>
struct TaskAwaiter {
  explicit TaskAwaiter(Task<R> &&task) noexcept;
  TaskAwaiter(TaskAwaiter &&completion) noexcept;
  TaskAwaiter(TaskAwaiter &) = delete;
  TaskAwaiter &operator=(TaskAwaiter &) = delete;
  constexpr bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> handle) noexcept;
  R await_resume() noexcept;
  Task<R> task;
};
template <typename ResultType>
struct TaskPromise {
  std::suspend_never initial_suspend() { return {}; }
  std::suspend_always final_suspend() noexcept { return {}; }
  Task<ResultType> get_return_object() {
    return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
  }
  void unhandled_exception() {
    std::lock_guard lock(completion_lock);
    result = Result<ResultType>(std::current_exception());
    completion.notify_all();
    notify_callbacks();
  }
  void return_value(ResultType value) {
    std::lock_guard lock(completion_lock);
    result = Result<ResultType>(std::move(value));
    completion.notify_all();
    notify_callbacks();
  }
  ResultType get_result() {
    std::unique_lock lock(completion_lock);
    if (!result.has_value()) {
      completion.wait(lock);
    }
    return result->get_or_throw();
  }
  void on_completed(std::function<void(Result<ResultType>)> &&func) {
    std::unique_lock lock(completion_lock);
    if (result.has_value()) {
      auto value = result.value();
      lock.unlock();
      func(value);
    } else {
      completion_callbacks.push_back(func);
    }
  }
  template <typename _ResultType>
  TaskAwaiter<_ResultType> await_transform(Task<_ResultType> &&task) {
    return TaskAwaiter<_ResultType>(std::move(task));
  }
  std::mutex completion_lock;
  std::condition_variable completion;
  std::optional<Result<ResultType>> result;
  std::list<std::function<void(Result<ResultType>)>> completion_callbacks;
  void notify_callbacks() {
    auto value = result.value();
    for (auto &callback : completion_callbacks) {
      callback(value);
    }
    completion_callbacks.clear();
  }
};
template <>
struct TaskPromise<void> {
  std::suspend_never initial_suspend() { return {}; }
  std::suspend_always final_suspend() noexcept { return {}; }
  Task<void> get_return_object();
  void unhandled_exception() {
    std::lock_guard lock(completion_lock);
    result = Result<void>(std::current_exception());
    completion.notify_all();
    notify_callbacks();
  }
  void get_result() {
    std::unique_lock lock(completion_lock);
    if (!result.has_value()) {
      completion.wait(lock);
    }
    result->get_or_throw();
  }
  void return_void() {
    std::lock_guard lock(completion_lock);
    result = Result<void>();
    completion.notify_all();
    notify_callbacks();
  }
  void on_completed(std::function<void(Result<void>)> &&func) {
    std::unique_lock lock(completion_lock);
    if (result.has_value()) {
      auto value = result.value();
      lock.unlock();
      func(value);
    } else {
      completion_callbacks.push_back(func);
    }
  }
  TaskAwaiter<void> await_transform(Task<void> &&task) {
    return TaskAwaiter<void>(std::move(task));
  }
  std::mutex completion_lock;
  std::condition_variable completion;
  std::optional<Result<void>> result;
  std::list<std::function<void(Result<void>)>> completion_callbacks;
  void notify_callbacks() {
    auto value = result.value();
    for (auto &callback : completion_callbacks) {
      callback(value);
    }
    completion_callbacks.clear();
  }
};
template <typename ResultType>
struct Task {
  using promise_type = TaskPromise<ResultType>;
  ResultType get_result() { return handle.promise().get_result(); }
  Task &then(std::function<void(ResultType)> &&func) {
    handle.promise().on_completed([func](auto result) {
      try {
        func(result.get_or_throw());
      } catch (std::exception &e) {
      }
    });
    return *this;
  }
  Task &catching(std::function<void(std::exception &)> &&func) {
    handle.promise().on_completed([func](auto result) {
      try {
        result.get_or_throw();
      } catch (std::exception &e) {
        func(e);
      }
    });
    return *this;
  }
  Task &finally(std::function<void()> &&func) {
    handle.promise().on_completed([func](auto result) { func(); });
    return *this;
  }
  explicit Task(std::coroutine_handle<promise_type> handle) noexcept
      : handle(handle) {}
  Task(Task &&task) noexcept : handle(std::exchange(task.handle, {})) {}
  Task(Task &) = delete;
  Task &operator=(Task &) = delete;
  ~Task() {
    if (handle) handle.destroy();
  }
  std::coroutine_handle<promise_type> handle;
};
template <>
struct Task<void> {
  using promise_type = TaskPromise<void>;
  void get_result() { handle.promise().get_result(); }
  Task &then(std::function<void()> &&func) {
    handle.promise().on_completed([func](auto result) {
      try {
        result.get_or_throw();
        func();
      } catch (std::exception &e) {
      }
    });
    return *this;
  }
  Task &catching(std::function<void(std::exception &)> &&func) {
    handle.promise().on_completed([func](auto result) {
      try {
        result.get_or_throw();
      } catch (std::exception &e) {
        func(e);
      }
    });
    return *this;
  }
  Task &finally(std::function<void()> &&func) {
    handle.promise().on_completed([func](auto result) { func(); });
    return *this;
  }
  explicit Task(std::coroutine_handle<promise_type> handle) noexcept
      : handle(handle) {}
  Task(Task &&task) noexcept : handle(std::exchange(task.handle, {})) {}
  Task(Task &) = delete;
  Task &operator=(Task &) = delete;
  ~Task() {
    if (handle) handle.destroy();
  }
  std::coroutine_handle<promise_type> handle;
};
}  // namespace Coroutine