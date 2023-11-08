#include "../include/BaseObject.h"

std::unordered_map<std::string, std::vector<BaseObject*>>
BaseObject::BaseObjects;

void BaseObject::OnCreate() {}
void BaseObject::OnStart() {}
void BaseObject::OnUpdate() {}
void BaseObject::OnStop() {}
void BaseObject::OnDestroy() {}
