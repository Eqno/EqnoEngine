#pragma once

#include <glm/mat4x4.hpp>

namespace MathUtils {
	glm::mat4x4 MakeScaleMatrix(const glm::vec3& scale);
	glm::mat4x4 MakeRotationMatrix(const glm::vec3& rotation);
	glm::mat4x4 MakeTranslationMatrix(const glm::vec3& translation);
	glm::mat4x4 MakeTransformMatrix(const glm::vec3& scale,
		const glm::vec3& rotation,
		const glm::vec3& translation);
}
