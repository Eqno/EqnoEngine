#include "../include/BaseTransform.h"

#include "Engine/Utility/include/MathUtils.h"
#include "Engine/Scene/include/SceneObject.h"

void BaseTransform::UpdateAbsolutePosition() {
	absolutePosition = relativePosition;
	if (const SceneObject* parent = _owner->GetParent(); parent != nullptr) {
		absolutePosition += parent->GetTransform().GetAbsolutePosition();
	}
	UpdateRelativeTransform();
	UpdateAbsoluteTransform();
}

void BaseTransform::UpdateAbsoluteRotation() {
	absoluteRotation = relativeRotation;
	if (const SceneObject* parent = _owner->GetParent(); parent != nullptr) {
		absoluteRotation += parent->GetTransform().GetAbsoluteRotation();
	}
	UpdateRelativeTransform();
	UpdateAbsoluteTransform();
}

void BaseTransform::UpdateAbsoluteScale() {
	absoluteScale = relativeScale;
	if (const SceneObject* parent = _owner->GetParent(); parent != nullptr) {
		absoluteScale *= parent->GetTransform().GetAbsoluteScale();
	}
	UpdateRelativeTransform();
	UpdateAbsoluteTransform();
}

void BaseTransform::UpdateRelativePosition() {
	relativePosition = absolutePosition;
	if (const SceneObject* parent = _owner->GetParent(); parent != nullptr) {
		relativePosition -= parent->GetTransform().GetAbsolutePosition();
	}
	UpdateRelativeTransform();
	UpdateAbsoluteTransform();
}

void BaseTransform::UpdateRelativeRotation() {
	relativeRotation = absoluteRotation;
	if (const SceneObject* parent = _owner->GetParent(); parent != nullptr) {
		relativeRotation -= parent->GetTransform().GetAbsoluteRotation();
	}
	UpdateRelativeTransform();
	UpdateAbsoluteTransform();
}

void BaseTransform::UpdateRelativeScale() {
	relativeScale = absoluteScale;
	if (const SceneObject* parent = _owner->GetParent(); parent != nullptr) {
		relativeScale /= parent->GetTransform().GetAbsoluteScale();
	}
	UpdateRelativeTransform();
	UpdateAbsoluteTransform();
}

void BaseTransform::UpdateRelativeTransform() {
	relativeTransform = MathUtils::MakeTransformMatrix(relativeScale,
		relativeRotation, relativePosition);
}

void BaseTransform::UpdateAbsoluteTransform() {
	absoluteTransform = MathUtils::MakeTransformMatrix(absoluteScale,
		absoluteRotation, absolutePosition);
}

glm::vec3 BaseTransform::GetRelativePosition() const {
	return relativePosition;
}

glm::vec3 BaseTransform::GetAbsolutePosition() const {
	return absolutePosition;
}

glm::vec3 BaseTransform::GetRelativeRotation() const {
	return relativeRotation;
}

glm::vec3 BaseTransform::GetAbsoluteRotation() const {
	return absoluteRotation;
}

glm::vec3 BaseTransform::GetRelativeScale() const {
	return relativeScale;
}

glm::vec3 BaseTransform::GetAbsoluteScale() const {
	return absoluteScale;
}

glm::vec3 BaseTransform::GetRelativeForward() const {
	return relativeForward;
}

glm::vec3 BaseTransform::GetAbsoluteForward() const {
	return absoluteForward;
}

glm::vec3 BaseTransform::GetRelativeUp() const {
	return relativeUp;
}

glm::vec3 BaseTransform::GetAbsoluteUp() const {
	return absoluteUp;
}

glm::vec3 BaseTransform::GetRelativeRight() const {
	return relativeRight;
}

glm::vec3 BaseTransform::GetAbsoluteRight() const {
	return absoluteRight;
}

glm::mat4x4 BaseTransform::GetRelativeTransform() const {
	return relativeTransform;
}

glm::mat4x4 BaseTransform::GetAbsoluteTransform() const {
	return absoluteTransform;
}

void BaseTransform::SetRelativePosition(const glm::vec3& position) {
	relativePosition = position;
	UpdateAbsolutePosition();
}

void BaseTransform::SetAbsolutePosition(const glm::vec3& position) {
	absolutePosition = position;
	UpdateRelativePosition();
}

void BaseTransform::SetRelativeRotation(const glm::vec3& rotation) {
	relativeRotation = rotation;
	UpdateAbsoluteRotation();
}

void BaseTransform::SetAbsoluteRotation(const glm::vec3& rotation) {
	absoluteRotation = rotation;
	UpdateRelativeRotation();
}

void BaseTransform::SetRelativeScale(const glm::vec3& scale) {
	relativeScale = scale;
	UpdateAbsoluteScale();
}

void BaseTransform::SetAbsoluteScale(const glm::vec3& scale) {
	absoluteScale = scale;
	UpdateRelativeScale();
}
