#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

class SceneObject;

class BaseTransform {
	glm::vec3 relativeForward;
	glm::vec3 absoluteForward;

	glm::vec3 relativeRight;
	glm::vec3 absoluteRight;

	glm::vec3 relativeUp;
	glm::vec3 absoluteUp;

	glm::vec3 relativePosition;
	glm::vec3 absolutePosition;

	SceneObject* _owner = nullptr;

public:
	void RegisterOwner(SceneObject* other) {
		_owner = other;
	}

	
};
