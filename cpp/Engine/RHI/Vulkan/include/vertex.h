#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

using AttributeDescriptions = std::array<VkVertexInputAttributeDescription, 5>;

class Vertex {
	glm::vec3 pos {};
	glm::vec4 color {};
	glm::vec3 normal {};
	glm::vec3 tangent {};
	glm::vec2 texCoord {};

public:
	Vertex(const glm::vec3& pos,
		const glm::vec4& color,
		const glm::vec3& normal,
		const glm::vec3& tangent,
		const glm::vec2& texCoord) : pos(pos),
	color(color),
	normal(normal),
	tangent(tangent),
	texCoord(texCoord) { }

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && normal == other.
		       normal && tangent == other.tangent && texCoord == other.texCoord;
	}

	struct HashFunction {
		size_t operator()(const Vertex& vertex) const {
			size_t posHash = std::hash<float>()(vertex.pos[0]);
			posHash ^= std::hash<float>()(vertex.pos[1]) << 1;
			posHash ^= std::hash<float>()(vertex.pos[2]) << 2;

			size_t colorHash = std::hash<float>()(vertex.color[0]);
			colorHash ^= std::hash<float>()(vertex.color[1]) << 1;
			colorHash ^= std::hash<float>()(vertex.color[2]) << 2;

			size_t normalHash = std::hash<float>()(vertex.normal[0]);
			normalHash ^= std::hash<float>()(vertex.normal[1]) << 1;
			normalHash ^= std::hash<float>()(vertex.normal[2]) << 2;

			size_t tangentHash = std::hash<float>()(vertex.tangent[0]);
			tangentHash ^= std::hash<float>()(vertex.tangent[1]) << 1;
			tangentHash ^= std::hash<float>()(vertex.tangent[2]) << 2;

			size_t texHash = std::hash<float>()(vertex.texCoord[0]);
			texHash ^= std::hash<float>()(vertex.texCoord[1]) << 1;
			return posHash ^ colorHash ^ normalHash ^ tangentHash ^ texHash;
		}
	};

	static VkVertexInputBindingDescription GetBindingDescription();
	static AttributeDescriptions GetAttributeDescriptions();
};
