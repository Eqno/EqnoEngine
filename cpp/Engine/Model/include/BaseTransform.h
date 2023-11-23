#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

class SceneObject;

class BaseTransform {
	glm::mat4x4 _relativeTransform = glm::mat4x4(1);
	glm::mat4x4 _absoluteTransform = glm::mat4x4(1);

	SceneObject* _owner = nullptr;

public:
	void RegisterOwner(SceneObject* other) {
		_owner = other;
	}

	[[nodiscard]] glm::vec3 GetRelativePosition() const {
		return _relativeTransform[3];
	}

	[[nodiscard]] glm::vec3 GetAbsolutePosition() const {
		return _absoluteTransform[3];
	}

	[[nodiscard]] glm::vec3 GetRelativeRotation() const {
		return eulerAngles(glm::quat(_relativeTransform));
	}

	[[nodiscard]] glm::vec3 GetAbsoluteRotation() const {
		return eulerAngles(glm::quat(_absoluteTransform));
	}

	[[nodiscard]] glm::vec3 GetRelativeScale() const {
		return _relativeTransform[0];
	}

	[[nodiscard]] glm::vec3 GetAbsoluteScale() const {
		return _absoluteTransform[0];
	}

	[[nodiscard]] glm::mat4x4 GetRelativeTransform() const {
		return _relativeTransform;
	}

	[[nodiscard]] glm::mat4x4 GetAbsoluteTransform() const {
		return _absoluteTransform;
	}

	void SetRelativePosition(const glm::vec3& position) {
		for (int i=0; i<3; i++) {
			_relativeTransform[3][i] = position[i];
		}
	}
};
