#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Engine/Utility/include/JsonUtils.h"

class Application;
class BaseObject {
  friend class Application;

  bool _alive;
  static std::unordered_map<std::string, std::unordered_set<BaseObject*>>
      _BaseObjects;

 protected:
  BaseObject* _owner;
  static std::unordered_map<std::string, std::unordered_set<BaseObject*>>
      BaseObjects;

  std::string _root = "Unset";
  std::string _file = "Unset";
  std::string _name = "Unset";

  BaseObject(BaseObject* owner, std::string root, std::string file)
      : _alive(true),
        _owner(owner),
        _root(std::move(root)),
        _file(std::move(file)),
        _name(JSON_CONFIG(String, "Name")) {}

  template <
      typename T, typename... Args,
      typename std::enable_if<std::is_base_of<BaseObject, T>{}, int>::type = 0>
  T* Create(const Args&... args) {
    T* ret = new T(this, args...);
    _BaseObjects[ret->_name].insert(ret);
    ret->OnCreate();
    return ret;
  }

 public:
  BaseObject() = delete;
  virtual ~BaseObject() {}

  template <
      typename T, typename... Args,
      typename std::enable_if<std::is_base_of<BaseObject, T>{}, int>::type = 0>
  static T* CreateImmediately(Args&&... args) {
    T* ret = new T(std::forward<Args>(args)...);
    _BaseObjects[ret->_name].insert(ret);
    ret->OnCreate();
    return ret;
  }

  void Destroy() { _alive = false; }
  void DestroyImmediately() {
    if (this != nullptr) {
      OnStop();
      OnDestroy();
      delete this;
    }
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

  [[nodiscard]] BaseObject* GetOwner() const { return _owner; }

  [[nodiscard]] const std::string& GetName() const { return _name; }

  [[nodiscard]] const std::string& GetRoot() const { return _root; }

  [[nodiscard]] const std::string& GetFile() const { return _file; }

  static const BaseObject* GetObjectByName(const std::string& name) {
    const auto& objects = BaseObjects[name];
    return objects.empty() ? nullptr : *objects.begin();
  }

  [[nodiscard]] static const std::unordered_set<BaseObject*>& GetObjectsByName(
      const std::string& name) {
    return BaseObjects[name];
  }
};