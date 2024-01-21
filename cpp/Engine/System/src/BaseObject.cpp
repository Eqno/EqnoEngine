#include "../include/BaseObject.h"

float BaseObject::DeltaTime = 0;
std::unordered_map<std::string, std::list<BaseObject*>>
    BaseObject::_BaseObjects, BaseObject::BaseObjects;

void BaseObject::OnCreate() {}
void BaseObject::OnStart() {}
void BaseObject::OnUpdate() {}
void BaseObject::OnStop() {}
void BaseObject::OnDestroy() {}

void BaseObject::OnActive() {}
void BaseObject::OnDeactive() {}
