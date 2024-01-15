#include "../include/BaseObject.h"

std::unordered_map<std::string, std::list<BaseObject*>>
    BaseObject::_BaseObjects, BaseObject::BaseObjects;

void BaseObject::OnCreate() {}
void BaseObject::OnStart() {}
void BaseObject::OnUpdate() {}
void BaseObject::OnStop() {}
void BaseObject::OnDestroy() {}
