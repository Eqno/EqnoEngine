#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include "utils.h"

using AttributeDescriptions = std::array<VkVertexInputAttributeDescription, 2>;

class Vertex {
	glm::vec2 pos {};
	glm::vec3 color {};

public:
	Vertex(const glm::vec2& pos, const glm::vec3& color) : pos(pos),
		color(color) {}

	static VkVertexInputBindingDescription GetBindingDescription();
	static AttributeDescriptions GetAttributeDescriptions();
};

const std::vector<uint16_t> INDICES = { 0, 1, 2, 2, 3, 0 };

const Vertexes VERTICES {
	{ { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
	{ { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
	{ { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
};
