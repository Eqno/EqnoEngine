#include "../include/mesh.h"

void Mesh::CreateMesh(const Device& device, const Render& render,
                  const MeshData* inData,
                  const VkDescriptorSetLayout& descriptorSetLayout) {
  _meshData = inData;

  ParseTextures(device, render);
  ParseVertexAndIndex();
  ParseBufferAndDescriptor(device, render, descriptorSetLayout);
}

void Mesh::DestroyMesh(const VkDevice& device, const Render& render) const {
  descriptor.DestroyDesciptor(device, render);
  buffer.DestroyBuffers(device);
  for (const Texture& texture : textures) {
    texture.DestroyTexture(device);
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
  data.CreateData(_meshData->indices, vertices);
}

void Mesh::ParseBufferAndDescriptor(
    const Device& device, const Render& render,
    const VkDescriptorSetLayout& descriptorSetLayout) {
  buffer.CreateBuffers(device, data.GetVertices(), data.GetIndices(), render);
  descriptor.CreateDescriptor(device, render, descriptorSetLayout, textures);
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