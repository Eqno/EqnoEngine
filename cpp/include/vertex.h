#pragma once

#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan_core.h>

#include "utils.h"

using AttributeDescriptions = std::array<VkVertexInputAttributeDescription, 2>;

class Vertex {
	glm::vec2 pos {};
	glm::vec3 color {};
public:
	Vertex(const glm::vec2& pos, const glm::vec3& color) : pos(pos),
		color(color) {}

	static auto GetBindingDescription() -> VkVertexInputBindingDescription;
	static auto GetAttributeDescriptions() -> AttributeDescriptions;
};

const Vertexes vertices{
	{ { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
	{ { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
	{ { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
};