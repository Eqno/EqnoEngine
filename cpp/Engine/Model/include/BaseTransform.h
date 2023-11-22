#pragma once

#include <glm/mat4x4.hpp>

class SceneObject;

class BaseTransform {
	glm::vec3 relativePosition = glm::vec3(0);
	glm::vec3 absolutePosition = glm::vec3(0);

	glm::vec3 relativeRotation = glm::vec3(0);
	glm::vec3 absoluteRotation = glm::vec3(0);

	glm::vec3 relativeScale = glm::vec3(1);
	glm::vec3 absoluteScale = glm::vec3(1);

	glm::vec3 relativeForward = glm::vec3(0, 0, 1);
	glm::vec3 absoluteForward = glm::vec3(0, 0, 1);

	glm::vec3 relativeUp = glm::vec3(0, 1, 0);
	glm::vec3 absoluteUp = glm::vec3(0, 1, 0);

	glm::vec3 relativeRight = glm::vec3(1, 0, 0);
	glm::vec3 absoluteRight = glm::vec3(1, 0, 0);

	glm::mat4x4 relativeTransform = glm::mat4x4(1);
	glm::mat4x4 absoluteTransform = glm::mat4x4(1);

	const SceneObject* _owner = nullptr;

	void UpdateAbsolutePosition();
	void UpdateAbsoluteRotation();
	void UpdateAbsoluteScale();

	void UpdateRelativePosition();
	void UpdateRelativeRotation();
	void UpdateRelativeScale();

	void UpdateRelativeTransform();
	void UpdateAbsoluteTransform();

public:
	[[nodiscard]] glm::vec3 GetRelativePosition() const;
	[[nodiscard]] glm::vec3 GetAbsolutePosition() const;

	[[nodiscard]] glm::vec3 GetRelativeRotation() const;
	[[nodiscard]] glm::vec3 GetAbsoluteRotation() const;

	[[nodiscard]] glm::vec3 GetRelativeScale() const;
	[[nodiscard]] glm::vec3 GetAbsoluteScale() const;

	[[nodiscard]] glm::vec3 GetRelativeForward() const;
	[[nodiscard]] glm::vec3 GetAbsoluteForward() const;

	[[nodiscard]] glm::vec3 GetRelativeUp() const;
	[[nodiscard]] glm::vec3 GetAbsoluteUp() const;

	[[nodiscard]] glm::vec3 GetRelativeRight() const;
	[[nodiscard]] glm::vec3 GetAbsoluteRight() const;

	[[nodiscard]] glm::mat4x4 GetRelativeTransform() const;
	[[nodiscard]] glm::mat4x4 GetAbsoluteTransform() const;

	void SetRelativePosition(const glm::vec3& position);
	void SetAbsolutePosition(const glm::vec3& position);

	void SetRelativeRotation(const glm::vec3& rotation);
	void SetAbsoluteRotation(const glm::vec3& rotation);

	void SetRelativeScale(const glm::vec3& scale);
	void SetAbsoluteScale(const glm::vec3& scale);

	void RegisterOwner(const SceneObject* owner) {
		_owner = owner;
	}
};
