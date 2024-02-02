#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "base.h"
#include "vertex.h"

class Data : public Base {
  std::vector<uint32_t> indices;
  std::vector<Vertex> vertices;

 public:
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

  void AddIndex(const uint32_t index) { indices.emplace_back(index); }

  void AddVertex(const Vertex& vertex) { vertices.emplace_back(vertex); }

  template <typename... Args>
  explicit Data(Base* owner, Args&&... args) : Base(owner) {
    CreateData(std::forward(args)...);
  }
  Data() = default;
  ~Data() override = default;

  void CreateData(const std::vector<uint32_t>& indices,
                  const std::vector<Vertex>& vertices);
};
