#include "vertex.h"

auto Vertex::GetBindingDescription() -> VkVertexInputBindingDescription {
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

auto Vertex::GetAttributeDescriptions() -> std::array<
	VkVertexInputAttributeDescription, 2> {
	const std::array attributeDescriptions {
		VkVertexInputAttributeDescription {
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, pos),
		},
		VkVertexInputAttributeDescription {
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, color),
		}
	};
	return attributeDescriptions;
}
