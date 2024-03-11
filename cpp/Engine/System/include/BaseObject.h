#pragma once

#include <Engine/Utility/include/JsonUtils.h>
#include <Engine/Utility/include/TypeUtils.h>

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

class Application;
class BaseObject : public std::enable_shared_from_this<BaseObject> {
  friend class Application;

  static std::list<std::shared_ptr<BaseObject>> PassiveObjects;
  static std::list<std::shared_ptr<BaseObject>> ActiveObjects;

 protected:
  static float DeltaTime;
  bool _alive, _active, _locked;
  std::weak_ptr<BaseObject> _owner;

  std::string _root = "Unset";
  std::string _file = "Unset";
  std::string _name = "Unset";

  BaseObject(std::string root, std::string file,
             std::weak_ptr<BaseObject> owner)
      : _alive(true),
        _active(true),
        _locked(false),
        _owner(owner),

        _root(std::move(root)),
        _file(std::move(file)),
        _name(JSON_CONFIG(String, "Name")) {}

  BaseObject(bool active, std::string root, std::string file,
             std::weak_ptr<BaseObject> owner)
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
  static std::shared_ptr<T> CreateImmediately(Args&&... args) {
    std::shared_ptr<T> ret = std::make_shared<T>(std::forward<Args>(args)...);
    if (ret->_active == true) {
      PassiveObjects.emplace_back(ret);
    }
    ret->OnCreate();
    return ret;
  }

  template <
      typename T, typename... Args,
      typename std::enable_if<std::is_base_of<BaseObject, T>{}, int>::type = 0>
  std::shared_ptr<T> Create(Args&&... args) {
    return CreateImmediately<T>(std::forward<Args>(args)...,
                                shared_from_this());
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
    if (_alive == true && _active == false) {
      OnDestroy();
    }
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

  [[nodiscard]] std::weak_ptr<BaseObject> GetOwner() const { return _owner; }
  [[nodiscard]] const std::string& GetName() const { return _name; }
  [[nodiscard]] const std::string& GetRoot() const { return _root; }
  [[nodiscard]] const std::string& GetFile() const { return _file; }
};
