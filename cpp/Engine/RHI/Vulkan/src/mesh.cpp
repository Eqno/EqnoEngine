#include "../include/mesh.h"

#include "../include/utils.h"

void Mesh::CreateMesh(
    const Device& device, const Render& render, std::weak_ptr<MeshData> inData,
    const VkDescriptorSetLayout& colorDescriptorSetLayout,
    const VkDescriptorSetLayout& zPrePassDescriptorSetLayout,
    const VkDescriptorSetLayout& shadowMapDescriptorSetLayout) {
  bridge = inData;

  ParseTextures(device, render);
  ParseVertexAndIndex();
  ParseBufferAndDescriptor(device, render, colorDescriptorSetLayout,
                           zPrePassDescriptorSetLayout,
                           shadowMapDescriptorSetLayout);
}

void Mesh::DestroyMesh(const VkDevice& device, const Render& render) {
  descriptor.DestroyDesciptor(device, render);
  buffer.DestroyBuffers(device);
  for (const Texture& texture : textures) {
    texture.DestroyTexture(device);
  }
}

void Mesh::ParseTextures(const Device& device, const Render& render) {
  if (auto bridgePtr = bridge.lock()) {
    for (const auto& [type, width, height, channels, _data] :
         bridgePtr->textures) {
      textures.emplace_back(VulkanUtils::TextureFormat[type], device, render,
                            width, height, channels, _data);
    }
  }
}

void Mesh::ParseVertexAndIndex() {
  std::vector<Vertex> vertices;

  if (auto bridgePtr = bridge.lock()) {
    vertices.reserve(bridgePtr->vertices.size());
    for (const auto& vert : bridgePtr->vertices) {
      vertices.emplace_back(vert);
    }
    data.CreateData(bridgePtr->indices, vertices);
  }
}

void Mesh::ParseBufferAndDescriptor(
    const Device& device, const Render& render,
    const VkDescriptorSetLayout& colorDescriptorSetLayout,
    const VkDescriptorSetLayout& zPrePassDescriptorSetLayout,
    const VkDescriptorSetLayout& shadowMapDescriptorSetLayout) {
  buffer.CreateBuffers(device, data.GetVertices(), data.GetIndices(), render);
  descriptor.CreateDescriptor(
      device, render, textures, colorDescriptorSetLayout,
      zPrePassDescriptorSetLayout, shadowMapDescriptorSetLayout);
}