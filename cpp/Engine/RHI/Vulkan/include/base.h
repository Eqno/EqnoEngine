#pragma once

#include <iostream>

class Base {
 protected:
  Base* owner;
  Base(Base* owner) : owner(owner) {}

 public:
  Base() = default;
  virtual ~Base() = default;

  template <typename T, typename... Args,
            typename std::enable_if<std::is_base_of<Base, T>{}, int>::type = 0>
  T* Create(Args&&... args) {
    T* ret = new T(this);
    ret->TriggerRegisterMember();
    ret->TriggerInitComponent(std::forward<Args>(args)...);
    return ret;
  }
  virtual void Destroy() { delete this; }
  void RegisterOwner(Base* owner) { this->owner = owner; }

  template <typename T>
  void _RegisterMember(T&& arg) {
    arg.RegisterOwner(this);
    arg.TriggerRegisterMember();
    arg.TriggerInitComponent();
  }

  template <typename... Args>
  void RegisterMember(Args&&... args) {
    (_RegisterMember(args), ...);
  }
  virtual void TriggerRegisterMember() {}
  virtual void TriggerInitComponent() {}
};