#include "../include/SceneObject.h"

void SceneObject::OnCreate() {
	BaseObject::OnCreate();
	transform.RegisterOwner(this);
}
