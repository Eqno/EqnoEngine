#pragma once

#include <Engine/Utility/include/JsonUtils.h>
#include <Engine/Utility/include/TypeUtils.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

class Application;
class BaseObject {
  friend class Application;

  bool _alive, _active, _locked;
  static std::unordered_map<std::string, std::list<BaseObject*>> _BaseObjects;

 protected:
  BaseObject* _owner;
  static float DeltaTime;
  static std::unordered_map<std::string, std::list<BaseObject*>> BaseObjects;

  std::string _root = "Unset";
  std::string _file = "Unset";
  std::string _name = "Unset";

  BaseObject(std::string root, std::string file, BaseObject* owner)
      : _alive(true),
        _active(true),
        _locked(false),
        _root(std::move(root)),
        _file(std::move(file)),
        _owner(owner),
        _name(JSON_CONFIG(String, "Name")) {}

  BaseObject(bool active, std::string root, std::string file, BaseObject* owner)
      : _alive(true),
        _active(false),
        _locked(false),
        _root(std::move(root)),
        _file(std::move(file)),
        _owner(owner),
        _name(JSON_CONFIG(String, "Name")) {}

  template <
      typename T, typename... Args,
      typename std::enable_if<std::is_base_of<BaseObject, T>{}, int>::type = 0>
  T* Create(Args&&... args) {
    T* ret = new T(std::forward<Args>(args)..., this);
    if (ret->_active == true) {
      _BaseObjects[ret->_name].emplace_back(ret);
    }
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
    _BaseObjects[ret->_name].emplace_back(ret);
    ret->OnCreate();
    return ret;
  }

  void Active() {
    if (_alive == true && _active == false) {
      _active = true;
      OnActive();
      if (_locked == false) {
        _BaseObjects[_name].emplace_back(this);
      }
    }
  }

  void Deactive() {
    if (_alive == true && _active == true) {
      _active = false;
    }
  }

  void Destroy() {
    _alive = false;
    _active = false;
  }
  void DestroyImmediately() {
    if (_active == true) {
      OnStop();
    }
    OnDestroy();
    delete this;
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

  static const BaseObject* GetObjectByName(const std::string& name) {
    const auto& objects = BaseObjects[name];
    return objects.empty() ? nullptr : *objects.begin();
  }

  [[nodiscard]] static const std::list<BaseObject*>& GetObjectsByName(
      const std::string& name) {
    return BaseObjects[name];
  }
};
