#include <Engine/RHI/Vulkan/include/draw.h>
#include <Engine/RHI/Vulkan/include/mesh.h>
#include <Engine/RHI/Vulkan/include/utils.h>

BufferManager& Mesh::GetBufferManager() const {
  return static_cast<Draw*>(owner)->GetBufferManager();
}

void Mesh::CreateMesh(
    const Device& device, Render& render, std::weak_ptr<MeshData> inData,
    const VkDescriptorSetLayout& colorDescriptorSetLayout,
    const VkDescriptorSetLayout& zPrePassDescriptorSetLayout,
    const VkDescriptorSetLayout& shadowMapDescriptorSetLayout) {
  bridge = inData;

  ParseTextures(device, render);
  if (createInterrupted) {
    return;
  }
  ParseVertexAndIndex();
  ParseBufferAndDescriptor(device, render, colorDescriptorSetLayout,
                           zPrePassDescriptorSetLayout,
                           shadowMapDescriptorSetLayout);
}

void Mesh::DestroyMesh(const VkDevice& device, const Render& render) {
  if (createInterrupted) {
    return;
  }
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
      Texture texture(VulkanUtils::TextureFormat[type], device, render, width,
                      height, channels, _data);
      if (texture.GetCreateInterrupted()) {
        createInterrupted = true;
        return;
      }
      textures.emplace_back(texture);
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
    const Device& device, Render& render,
    const VkDescriptorSetLayout& colorDescriptorSetLayout,
    const VkDescriptorSetLayout& zPrePassDescriptorSetLayout,
    const VkDescriptorSetLayout& shadowMapDescriptorSetLayout) {
  buffer.CreateBuffers(device, data.GetVertices(), data.GetIndices(), render);
  descriptor.CreateDescriptor(
      device, render, textures, colorDescriptorSetLayout,
      zPrePassDescriptorSetLayout, shadowMapDescriptorSetLayout);
}

PipelineBuffer* Mesh::GetPipelineBuffer() {
  return static_cast<Draw*>(owner)->GetPipelineBuffer();
}