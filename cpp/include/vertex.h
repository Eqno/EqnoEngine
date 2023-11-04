#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include "utils.h"

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

const std::vector<uint16_t> INDICES = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};

const std::vector<Vertex> VERTICES = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};
