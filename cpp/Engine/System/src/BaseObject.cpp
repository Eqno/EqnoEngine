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

void BaseObject::AddObjectToPassiveObjects(std::shared_ptr<BaseObject> inst) {
  if (inst->_active == true) {
    if (auto appPtr = dynamic_pointer_cast<Application>(inst)) {
      appPtr->_app = appPtr;
    }
    if (auto appPtr = inst->_app.lock()) {
      appPtr->passiveObjects.emplace_back(inst);
    }
  }
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
