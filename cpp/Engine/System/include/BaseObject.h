#pragma once

#include <Engine/Utility/include/JsonUtils.h>
#include <Engine/Utility/include/TypeUtils.h>

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

class Application;
class BaseObject {
  friend class Application;

  static std::list<BaseObject*> PassiveObjects;
  static std::list<BaseObject*> ActiveObjects;

 protected:
  static float DeltaTime;
  bool _alive, _active, _locked;
  BaseObject* _owner;

  std::string _root = "Unset";
  std::string _file = "Unset";
  std::string _name = "Unset";

  BaseObject(std::string root, std::string file, BaseObject* owner)
      : _alive(true),
        _active(true),
        _locked(false),
        _owner(owner),

        _root(std::move(root)),
        _file(std::move(file)),
        _name(JSON_CONFIG(String, "Name")) {}

  BaseObject(bool active, std::string root, std::string file, BaseObject* owner)
      : _alive(true),
        _active(active),
        _locked(false),
        _owner(owner),

        _root(std::move(root)),
        _file(std::move(file)),
        _name(JSON_CONFIG(String, "Name")) {}

 public:
  BaseObject() = delete;
  virtual ~BaseObject() {}

  template <
      typename T, typename... Args,
      typename std::enable_if<std::is_base_of<BaseObject, T>{}, int>::type = 0>
  static T* CreateImmediately(Args&&... args) {
    T* ret = new T(std::forward<Args>(args)...);
    if (ret->_active == true) {
      PassiveObjects.emplace_back(ret);
    }
    ret->OnCreate();
    return ret;
  }

  template <
      typename T, typename... Args,
      typename std::enable_if<std::is_base_of<BaseObject, T>{}, int>::type = 0>
  T* Create(Args&&... args) {
    return CreateImmediately<T>(std::forward<Args>(args)..., this);
  }

  void Active() {
    if (_alive == true && _active == false) {
      _active = true;
      OnActive();
      if (_locked == false) {
        ActiveObjects.emplace_back(this);
      }
    }
  }

  void Deactive() {
    if (_alive == true && _active == true) {
      _active = false;
      _locked = true;
    }
  }

  void Destroy() {
    _alive = false;
    _active = false;
  }

  BaseObject& operator=(const BaseObject& app) = delete;
  BaseObject& operator=(BaseObject&& app) = delete;

  BaseObject(const BaseObject& app) = delete;
  BaseObject(BaseObject&& app) = delete;

  virtual void OnCreate();
  virtual void OnStart();
  virtual void OnUpdate();
  virtual void OnStop();
  virtual void OnDestroy();

  virtual void OnActive();
  virtual void OnDeactive();

  [[nodiscard]] BaseObject* GetOwner() const { return _owner; }
  [[nodiscard]] const std::string& GetName() const { return _name; }
  [[nodiscard]] const std::string& GetRoot() const { return _root; }
  [[nodiscard]] const std::string& GetFile() const { return _file; }
};
