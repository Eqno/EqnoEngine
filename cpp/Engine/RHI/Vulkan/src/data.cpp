#include "../include/data.h"

#include <stdexcept>
#include <unordered_map>

#include "Engine/RHI/Vulkan/include/utils.h"

void Data::CreateData(const std::vector<uint32_t>& inIndices,
                      const std::vector<Vertex>& inVertices) {
  indices = inIndices;
  vertices = inVertices;
}
