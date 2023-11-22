#include "../include/MathUtils.h"

glm::mat4x4 MathUtils::MakeScaleMatrix(const glm::vec3& scale) {
	return glm::mat4x4 {
		{scale.x, 0, 0, 0},
		{0, scale.y, 0, 0},
		{0, 0, scale.z, 0},
		{0, 0, 0, 1}
	};
}

glm::mat4x4 MathUtils::MakeRotationMatrix(const glm::vec3& rotation) {
	return glm::mat4x4 {
		{1, 0, 0, 0},
		{0, cos(rotation.x), -sin(rotation.x), 0},
		{0, sin(rotation.x), cos(rotation.x), 0},
		{0, 0, 0, 1}
	} * glm::mat4x4 {
		{cos(rotation.x), 0, sin(rotation.x), 0},
		{0, 1, 0, 0},
		{-sin(rotation.x), 0, cos(rotation.x), 0},
		{0, 0, 0, 1}
	} * glm::mat4x4 {
		{cos(rotation.x), -sin(rotation.x), 0, 0},
		{sin(rotation.x), cos(rotation.x), 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}
	};
}

glm::mat4x4 MathUtils::MakeTranslationMatrix(const glm::vec3& translation) {
	return glm::mat4x4 {
		{1, 0, 0, translation.x},
		{0, 1, 0, translation.y},
		{0, 0, 1, translation.z},
		{0, 0, 0, 1}
	};
}

glm::mat4x4 MathUtils::MakeTransformMatrix(const glm::vec3& scale,
	const glm::vec3& rotation,
	const glm::vec3& translation) {
	return MakeScaleMatrix(scale) * MakeRotationMatrix(rotation) *
		MakeTranslationMatrix(translation);
}
