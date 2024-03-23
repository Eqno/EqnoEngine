#include <Engine/System/include/Application.h>
#include <Engine/System/include/BaseObject.h>

float BaseObject::DeltaTime;

void BaseObject::OnCreate() {}
void BaseObject::OnStart() {}
void BaseObject::OnUpdate() {}
void BaseObject::OnStop() {}
void BaseObject::OnDestroy() {}

void BaseObject::OnActive() {}
void BaseObject::OnDeactive() {}

template <typename T, typename... Args,
          typename std::enable_if<std::is_base_of<BaseObject, T>{}, int>::type>
static std::shared_ptr<T> BaseObject::CreateImmediately(Args&&... args) {
  std::shared_ptr<T> ret = std::make_shared<T>(std::forward<Args>(args)...);
  if (ret->_active == true) {
    if (auto appPtr = ret->_app.lock()) {
      appPtr->passiveObjects.emplace_back(ret);
    }
  }
  ret->OnCreate();
  return ret;
}

template <typename T, typename... Args,
          typename std::enable_if<std::is_base_of<BaseObject, T>{}, int>::type>
std::shared_ptr<T> BaseObject::Create(Args&&... args) {
  return CreateImmediately<T>(std::forward<Args>(args)..., shared_from_this());
}

void BaseObject::Active() {
  if (_alive == true && _active == false) {
    _active = true;
    OnActive();
    if (_locked == false) {
      if (auto appPtr = _app.lock()) {
        appPtr->activeObjects.emplace_back(this);
      }
    }
  }
}

void BaseObject::Deactive() {
  if (_alive == true && _active == true) {
    _active = false;
    _locked = true;
  }
}

void BaseObject::Destroy() {
  if (_alive == true && _active == false) {
    OnDestroy();
  }
  _alive = false;
  _active = false;
}
