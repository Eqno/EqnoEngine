#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

using AttributeDescriptions = std::array<VkVertexInputAttributeDescription, 3>;

class Vertex {
	glm::vec3 pos {};
	glm::vec3 color {};
	glm::vec2 texCoord {};

public:
	Vertex(const glm::vec3& pos,
		const glm::vec3& color,
		const glm::vec2 texCoord) : pos(pos), color(color), texCoord(texCoord) {
	}

	static VkVertexInputBindingDescription GetBindingDescription();
	static AttributeDescriptions GetAttributeDescriptions();
};
