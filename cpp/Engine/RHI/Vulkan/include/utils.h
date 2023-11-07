#pragma once

#include <stdexcept>
#include <string>
#include <vulkan/vulkan_core.h>

#include <assimp/scene.h>
#include <assimp/matrix4x4.h>

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class Vertex;

namespace VulkanUtils {
	inline VkFormat ParseImageFormat(const std::string& imageFormat) {
		if (imageFormat == "SRGB") {
			return VK_FORMAT_R8G8B8A8_SRGB;
		}
		return VK_FORMAT_R8G8B8A8_UNORM;
	}

	inline VkColorSpaceKHR ParseColorSpace(const std::string& colorSpace) {
		if (colorSpace == "SRGB") {
			return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		}
		return VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
	}
}

namespace MathUtils {
	inline glm::vec4 AiColor4D2GlmVec4(const aiColor4D vec) {
		return {vec.r, vec.g, vec.b, vec.a};
	}

	inline glm::vec3 AiVector3D2GlmVec3(const aiVector3D vec) {
		return {vec.x, vec.y, vec.z};
	}

	inline glm::vec2 AiVector2D2GlmVec2(const aiVector3D vec) {
		return {vec.x, vec.y};
	}
}
