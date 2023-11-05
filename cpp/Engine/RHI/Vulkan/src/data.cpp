#include "../include/data.h"

#include <stdexcept>
#include <tiny_obj_loader.h>
#include <unordered_map>

void Data::Create(const char* dataPath) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, dataPath)) {
		throw std::runtime_error(warn + err);
	}
	std::unordered_map<Vertex, uint32_t, Vertex::HashFunction> uniqueVertices
		{};

	for (const auto& [name, mesh, lines, points]: shapes) {
		for (const auto& [vertex_index, normal_index, texcoord_index]: mesh.
		     indices) {
			glm::vec3 pos = {
				attrib.vertices[3 * vertex_index + 0],
				attrib.vertices[3 * vertex_index + 1],
				attrib.vertices[3 * vertex_index + 2],
			};
			glm::vec3 color = {1.0f, 1.0f, 1.0f};
			glm::vec2 texCoord = {
				attrib.texcoords[2 * texcoord_index + 0],
				1.0f - attrib.texcoords[2 * texcoord_index + 1],
			};
			Vertex vertex(pos, color, texCoord);
			if (!uniqueVertices.contains(vertex)) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			indices.push_back(uniqueVertices[vertex]);
		}
	}
}