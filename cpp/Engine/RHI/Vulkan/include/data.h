#pragma once

#include <cstdint>
#include <vector>

#include "vertex.h"

class Data {
	std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};
	std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	};

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

	explicit Data(const char* dataPath) {
		Create((dataPath));
	}

	void Create(const char* dataPath);
};
