#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "vertex.h"

class Data {
	std::vector<uint32_t> indices {};
	std::vector<Vertex> vertices {};

public:
	Data() = default;

	Data(const std::vector<uint32_t>& indices,
		const std::vector<Vertex>& vertices) : indices(indices),
		vertices(vertices) {}

	[[nodiscard]] const std::vector<uint32_t>& GetIndices() const {
		return indices;
	}

	[[nodiscard]] const std::vector<Vertex>& GetVertices() const {
		return vertices;
	}

	[[nodiscard]] const uint32_t& GetIndexByIndex(const uint32_t index) const {
		return indices[index];
	}

	[[nodiscard]] const Vertex& GetVertexByIndex(const uint32_t index) const {
		return vertices[index];
	}

	void AddIndex(const uint32_t index) {
		indices.emplace_back(index);
	}

	void AddVertex(const Vertex& vertex) {
		vertices.emplace_back(vertex);
	}

	explicit Data(const std::string& objPath) {
		Create(objPath);
	}

	void Create(const std::string& objPath);

	void Create(const std::vector<uint32_t>& indices,
		const std::vector<Vertex>& vertices);
};
