#include "../include/buffer.h"

#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Light/include/LightChannel.h>
#include <Engine/Model/include/BaseMaterial.h>

#include <stdexcept>

#include "../include/device.h"
#include "../include/pipeline.h"
#include "../include/render.h"
#include "../include/uniform.h"

uint32_t DataBuffer::MemoryType(const VkPhysicalDevice& physicalDevice,
                                const glm::uint32_t& typeFilter,
                                const VkMemoryPropertyFlags& properties) {
  VkPhysicalDeviceMemoryProperties memProperties{};
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("Failed to find suitable memory type!");
}

void DataBuffer::CreateBuffer(const Device& device, const VkDeviceSize& size,
                              const VkBufferUsageFlags& usage,
                              const VkMemoryPropertyFlags& properties,
                              VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
  const VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  if (vkCreateBuffer(device.GetLogical(), &bufferInfo, nullptr, &buffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device.GetLogical(), buffer, &memRequirements);

  const VkMemoryAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = MemoryType(device.GetPhysical(),
                                    memRequirements.memoryTypeBits, properties),
  };
  if (vkAllocateMemory(device.GetLogical(), &allocInfo, nullptr,
                       &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate buffer memory!");
  }
  vkBindBufferMemory(device.GetLogical(), buffer, bufferMemory, 0);
}

void DataBuffer::CreateVertexBuffer(const Device& device,
                                    const std::vector<Vertex>& vertices,
                                    const Render& render) {
  const auto bufferSize =
      vertices.empty() ? 0 : sizeof(vertices[0]) * vertices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void* vertexData;
  vkMapMemory(device.GetLogical(), stagingBufferMemory, 0, bufferSize, 0,
              &vertexData);
  memcpy(vertexData, vertices.data(), bufferSize);
  vkUnmapMemory(device.GetLogical(), stagingBufferMemory);

  CreateBuffer(
      device, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
  render.CopyCommandBuffer(device, stagingBuffer, vertexBuffer, bufferSize);

  vkDestroyBuffer(device.GetLogical(), stagingBuffer, nullptr);
  vkFreeMemory(device.GetLogical(), stagingBufferMemory, nullptr);
}

void DataBuffer::CreateIndexBuffer(const Device& device,
                                   const std::vector<uint32_t>& indices,
                                   const Render& render) {
  const auto bufferSize =
      indices.empty() ? 0 : sizeof(indices[0]) * indices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void* indexData;
  vkMapMemory(device.GetLogical(), stagingBufferMemory, 0, bufferSize, 0,
              &indexData);
  memcpy(indexData, indices.data(), bufferSize);
  vkUnmapMemory(device.GetLogical(), stagingBufferMemory);

  CreateBuffer(
      device, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

  render.CopyCommandBuffer(device, stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(device.GetLogical(), stagingBuffer, nullptr);
  vkFreeMemory(device.GetLogical(), stagingBufferMemory, nullptr);
}

void DataBuffer::CreateBuffers(const Device& device,
                               const std::vector<Vertex>& vertices,
                               const std::vector<uint32_t>& indices,
                               const Render& render) {
  CreateVertexBuffer(device, vertices, render);
  CreateIndexBuffer(device, indices, render);
}

void DataBuffer::DestroyBuffers(const VkDevice& device) const {
  vkDestroyBuffer(device, indexBuffer, nullptr);
  vkFreeMemory(device, indexBufferMemory, nullptr);

  vkDestroyBuffer(device, vertexBuffer, nullptr);
  vkFreeMemory(device, vertexBufferMemory, nullptr);
}

void UniformBuffer::CreateUniformBuffer(const Device& device,
                                        const Render& render,
                                        unsigned long long bufferSize) {
  uniformBuffers.resize(render.GetMaxFramesInFlight());
  uniformBuffersMemory.resize(render.GetMaxFramesInFlight());
  uniformBuffersMapped.resize(render.GetMaxFramesInFlight());

  for (int i = 0; i < render.GetMaxFramesInFlight(); i++) {
    DataBuffer::CreateBuffer(device, bufferSize,
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             uniformBuffers[i], uniformBuffersMemory[i]);
    vkMapMemory(device.GetLogical(), uniformBuffersMemory[i], 0, bufferSize, 0,
                &uniformBuffersMapped[i]);
  }
}
void UniformBuffer::DestroyUniformBuffer(const VkDevice& device,
                                         const Render& render) const {
  for (auto i = 0; i < render.GetMaxFramesInFlight(); i++) {
    vkDestroyBuffer(device, uniformBuffers[i], nullptr);
    vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
  }
}

void CameraBuffer::UpdateUniformBuffer(BaseCamera* camera,
                                       const uint32_t currentImage) {
  if (updateLock == false) {
    updateLock = true;
  } else {
    return;
  }
  CameraData* buffer =
      reinterpret_cast<CameraData*>(uniformBuffersMapped[currentImage]);
  buffer->pos = camera->GetAbsolutePosition();
  buffer->normal = camera->GetAbsoluteForward();
}

void MaterialBuffer::UpdateUniformBuffer(BaseMaterial* material,
                                         const uint32_t currentImage) {
  if (updateLock == false) {
    updateLock = true;
  } else {
    return;
  }
  MaterialData* buffer =
      reinterpret_cast<MaterialData*>(uniformBuffersMapped[currentImage]);
  buffer->color = material->GetColor();
  buffer->roughness = material->GetRoughness();
  buffer->metallic = material->GetMetallic();
}

void LightChannelBuffer::UpdateUniformBuffer(LightChannel* lightChannel,
                                             const uint32_t currentImage) {
  if (updateLock == false) {
    updateLock = true;
  } else {
    return;
  }
  LightChannelData* buffer =
      reinterpret_cast<LightChannelData*>(uniformBuffersMapped[currentImage]);

  std::vector<std::weak_ptr<BaseLight>>& lights = lightChannel->GetLights();
  buffer->num = lights.size();

  for (int i = 0; i < lights.size(); ++i) {
    if (auto lightPtr = lights[i].lock()) {
      buffer->object[i].id = lightPtr->GetId();
      buffer->object[i].type = lightPtr->GetType();
      buffer->object[i].intensity = lightPtr->GetIntensity();
      buffer->object[i].color = lightPtr->GetColor();

      buffer->object[i].pos = lightPtr->GetAbsolutePosition();
      buffer->object[i].normal = lightPtr->GetAbsoluteForward();

      buffer->object[i].viewMatrix = lightPtr->GetViewMatrix();
      buffer->object[i].projMatrix = lightPtr->GetProjMatrix();
    }
  }
}