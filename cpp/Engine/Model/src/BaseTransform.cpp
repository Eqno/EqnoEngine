#include "../include/BaseTransform.h"

#include "Engine/Utility/include/MathUtils.h"
#include "Engine/Scene/include/SceneObject.h"

void BaseTransform::UpdateAbsolutePosition() {
	SceneObject* parent = _owner->GetParent();
	if (parent != nullptr) {
		absolutePosition = {
			false,
			relativePosition.second.x * parent->GetTransform().
			                                    GetAbsoluteRight() +
			relativePosition.second.y * parent->GetTransform().GetAbsoluteUp() +
			relativePosition.second.z * parent->GetTransform().
			                                    GetAbsoluteForward() + parent->
			GetTransform().GetAbsolutePosition()
		};
	}
}

void BaseTransform::UpdateAbsoluteRotation() {}

void BaseTransform::UpdateAbsoluteScale() {}

void BaseTransform::UpdateRelativePosition() {}

void BaseTransform::UpdateRelativeRotation() {}

void BaseTransform::UpdateRelativeScale() {}

void BaseTransform::UpdateRelativeTransform() {}

void BaseTransform::UpdateAbsoluteTransform() {}

TRANSFORM_MEMBERS(IMPLEMENT_SOURCE)
