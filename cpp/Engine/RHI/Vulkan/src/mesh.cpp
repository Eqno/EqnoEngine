#include "../include/mesh.h"

void Mesh::Create(const Device& device, const Render& render,
                  const MeshData* inData,
                  const VkDescriptorSetLayout& descriptorSetLayout) {
  _meshData = inData;

  ParseTextures(device, render);
  ParseVertexAndIndex();
  ParseBufferAndDescriptor(device, render, descriptorSetLayout);
}

void Mesh::Destroy(const VkDevice& device, const Render& render) const {
  descriptor.Destroy(device, render);
  buffer.Destroy(device);
  for (const Texture& texture : textures) {
    texture.Destroy(device);
  }
}

void Mesh::ParseTextures(const Device& device, const Render& render) {
  for (const auto& [width, height, channels, _data] : _meshData->textures) {
    textures.emplace_back("NOSRPGB", device, render, width, height, channels,
                          _data);
  }
}

void Mesh::ParseVertexAndIndex() {
  std::vector<Vertex> vertices;
  vertices.reserve(_meshData->vertices.size());
  for (const auto& vert : _meshData->vertices) {
    vertices.emplace_back(vert);
  }
  data.Create(_meshData->indices, vertices);
}

void Mesh::ParseBufferAndDescriptor(
    const Device& device, const Render& render,
    const VkDescriptorSetLayout& descriptorSetLayout) {
  descriptor.RegisterOwner(this);
  buffer.Create(device, data.GetVertices(), data.GetIndices(), render);
  descriptor.Create(device, render, descriptorSetLayout, textures);
}

const glm::mat4x4* Mesh::GetModelMatrix() {
  return &_meshData->uniform.modelMatrix;
}
const glm::mat4x4* Mesh::GetViewMatrix() {
  return &_meshData->uniform.viewMatrix;
}
const glm::mat4x4* Mesh::GetProjMatrix() {
  return &_meshData->uniform.projMatrix;
}