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
		const glm::vec2& texCoord) : pos(pos),
	color(color),
	texCoord(texCoord) { }

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.
		       texCoord;
	}

	struct HashFunction {
		size_t operator()(const Vertex& vertex) const {
			size_t posHash = std::hash<float>()(vertex.pos[0]);
			posHash ^= std::hash<float>()(vertex.pos[1]) << 1;
			posHash ^= std::hash<float>()(vertex.pos[2]) << 2;

			size_t colorHash = std::hash<float>()(vertex.color[0]);
			colorHash ^= std::hash<float>()(vertex.color[1]) << 1;
			colorHash ^= std::hash<float>()(vertex.color[2]) << 2;

			size_t texHash = std::hash<float>()(vertex.texCoord[0]);
			texHash ^= std::hash<float>()(vertex.texCoord[1]) << 1;

			return posHash ^ colorHash ^ texHash;
		}
	};

	static VkVertexInputBindingDescription GetBindingDescription();
	static AttributeDescriptions GetAttributeDescriptions();
};
