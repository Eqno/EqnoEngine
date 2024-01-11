#include "../include/SceneObject.h"

void SceneObject::OnCreate() { transform.RegisterOwner(this); }