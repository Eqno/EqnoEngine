#include "../include/vertex.h"

VkVertexInputBindingDescription Vertex::GetBindingDescription() {
	return {
		.binding = 0,
		.stride = sizeof(Vertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
}

AttributeDescriptions Vertex::GetAttributeDescriptions() {
	return {
		VkVertexInputAttributeDescription {
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, pos),
		},
		VkVertexInputAttributeDescription {
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, color),
		},
		VkVertexInputAttributeDescription {
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, texCoord),
		}
	};
}
