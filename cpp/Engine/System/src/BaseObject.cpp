#include "../include/BaseObject.h"

float BaseObject::DeltaTime = 0;
std::list<BaseObject*> BaseObject::PassiveObjects, BaseObject::ActiveObjects;

void BaseObject::OnCreate() {}
void BaseObject::OnStart() {}
void BaseObject::OnUpdate() {}
void BaseObject::OnStop() {}
void BaseObject::OnDestroy() {}

void BaseObject::OnActive() {}
void BaseObject::OnDeactive() {}
