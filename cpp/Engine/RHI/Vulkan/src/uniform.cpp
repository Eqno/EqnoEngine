#include "../include/uniform.h"

#include <Engine/Light/include/BaseLight.h>

#include <chrono>
#include <stdexcept>

#include "../include/buffer.h"
#include "../include/device.h"
#include "../include/mesh.h"
#include "../include/render.h"
#include "../include/texture.h"
#include "../include/uniform.h"

#define AddDescriptorWrites(index, type)                                 \
  {                                                                      \
    VkDescriptorBufferInfo bufferInfo{                                   \
        .buffer = GetUniformBufferByIndex(i * UniformBufferNum + index), \
        .offset = 0,                                                     \
        .range = sizeof(type),                                           \
    };                                                                   \
    descriptorWrites[index] = {                                          \
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                 \
        .dstSet = descriptorSets[i],                                     \
        .dstBinding = index,                                             \
        .dstArrayElement = 0,                                            \
        .descriptorCount = 1,                                            \
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,             \
        .pBufferInfo = &bufferInfo,                                      \
    };                                                                   \
  }

#define CreateAndMapBuffers(index, type)                                      \
  {                                                                           \
    Buffer::CreateBuffer(device, sizeof(type),                                \
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,                  \
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |                \
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,            \
                         uniformBuffers[i * UniformBufferNum + index],        \
                         uniformBuffersMemory[i * UniformBufferNum + index]); \
    vkMapMemory(device.GetLogical(),                                          \
                uniformBuffersMemory[i * UniformBufferNum + index], 0,        \
                sizeof(type), 0,                                              \
                &uniformBuffersMapped[i * UniformBufferNum + index]);         \
  }

#define CopyMemoryOfBuffers(index, ptr)                                      \
  memcpy(uniformBuffersMapped[currentImage * UniformBufferNum + index], ptr, \
         sizeof(*ptr))

#define DoSomethingWithBuffers(action) \
  {                                    \
    action(0, CameraData);             \
    action(1, MaterialData);           \
    action(2, TransformData);          \
    action(3, LightsData);             \
  }

void Descriptor::CreateDescriptorSets(
    const VkDevice& device, const Render& render,
    const VkDescriptorSetLayout& descriptorSetLayout,
    const std::vector<Texture>& textures) {
  const std::vector layouts(render.GetMaxFramesInFlight(), descriptorSetLayout);

  const VkDescriptorSetAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptorPool,
      .descriptorSetCount =
          static_cast<uint32_t>(render.GetMaxFramesInFlight()),
      .pSetLayouts = layouts.data(),
  };

  descriptorSets.resize(render.GetMaxFramesInFlight());
  if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor sets!");
  }

  for (auto i = 0; i < render.GetMaxFramesInFlight(); i++) {
    std::vector<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.resize(textures.size() + UniformBufferNum);
    DoSomethingWithBuffers(AddDescriptorWrites);

    std::vector<VkDescriptorImageInfo> imageInfos;
    imageInfos.resize(textures.size());

    for (size_t j = 0; j < textures.size(); j++) {
      imageInfos[j] = {
          .sampler = textures[j].GetTextureSampler(),
          .imageView = textures[j].GetTextureImageView(),
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
      descriptorWrites[j + UniformBufferNum] = {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = descriptorSets[i],
          .dstBinding = static_cast<uint32_t>(j + UniformBufferNum),
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .pImageInfo = &imageInfos[j],
      };
    }
    vkUpdateDescriptorSets(device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void Descriptor::CreateDescriptorPool(const VkDevice& device,
                                      const Render& render,
                                      const size_t textureNum) {
  std::vector<VkDescriptorPoolSize> poolSizes;
  poolSizes.resize(textureNum + UniformBufferNum);

  for (int i = 0; i < UniformBufferNum; i++) {
    poolSizes[i].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[i].descriptorCount =
        static_cast<uint32_t>(render.GetMaxFramesInFlight());
  }
  for (size_t i = 0; i < textureNum; i++) {
    poolSizes[i + UniformBufferNum].type =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[i + UniformBufferNum].descriptorCount =
        static_cast<uint32_t>(render.GetMaxFramesInFlight());
  }

  const VkDescriptorPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = static_cast<uint32_t>(render.GetMaxFramesInFlight()),
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.data(),
  };

  if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

void Descriptor::CreateUniformBuffer(const Device& device,
                                     const Render& render) {
  uniformBuffer.CreateUniformBuffers(device, render);
}

void Descriptor::DestroyUniformBuffers(const VkDevice& device,
                                       const Render& render) const {
  uniformBuffer.DestroyUniformBuffer(device, render);
}

void Descriptor::CreateDescriptor(
    const Device& device, const Render& render,
    const VkDescriptorSetLayout& descriptorSetLayout,
    const std::vector<Texture>& textures) {
  CreateUniformBuffer(device, render);
  CreateDescriptorPool(device.GetLogical(), render, textures.size());
  CreateDescriptorSets(device.GetLogical(), render, descriptorSetLayout,
                       textures);
}

void Descriptor::DestroyDesciptor(const VkDevice& device,
                                  const Render& render) const {
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  DestroyUniformBuffers(device, render);
}

void UniformBuffer::CreateUniformBuffers(const Device& device,
                                         const Render& render) {
  uniformBuffers.resize(render.GetMaxFramesInFlight() * UniformBufferNum);
  uniformBuffersMemory.resize(render.GetMaxFramesInFlight() * UniformBufferNum);
  uniformBuffersMapped.resize(render.GetMaxFramesInFlight() * UniformBufferNum);

  for (int i = 0; i < render.GetMaxFramesInFlight(); i++) {
    DoSomethingWithBuffers(CreateAndMapBuffers);
  }
}

void UniformBuffer::UpdateUniformBuffer(const uint32_t currentImage) {
  {
    CameraData* buffer = reinterpret_cast<CameraData*>(
        uniformBuffersMapped[currentImage * UniformBufferNum]);
    buffer->pos = *GetBridgeData()->uniform.cameraPosition;
    buffer->normal = *GetBridgeData()->uniform.cameraForward;
  }
  CopyMemoryOfBuffers(1, GetBridgeData()->uniform.material);
  {
    TransformData* buffer = reinterpret_cast<TransformData*>(
        uniformBuffersMapped[currentImage * UniformBufferNum + 2]);
    buffer->modelMatrix = *GetBridgeData()->uniform.modelMatrix;
    buffer->viewMatrix = *GetBridgeData()->uniform.viewMatrix;
    buffer->projMatrix = *GetBridgeData()->uniform.projMatrix;
  }
  {
    LightsData* buffer = reinterpret_cast<LightsData*>(
        uniformBuffersMapped[currentImage * UniformBufferNum + 3]);

    if (GetBridgeData()->uniform.lights != nullptr) {
      std::vector<BaseLight*>& lights = *GetBridgeData()->uniform.lights;

      buffer->num = lights.size();
      for (int i = 0; i < lights.size(); ++i) {
        buffer->object[i].type = lights[i]->GetType();
        buffer->object[i].intensity = lights[i]->GetIntensity();
        buffer->object[i].color = lights[i]->GetColor();

        buffer->object[i].pos = lights[i]->GetAbsolutePosition();
        buffer->object[i].normal = lights[i]->GetAbsoluteForward();
      }
    }
  }
}

void UniformBuffer::DestroyUniformBuffer(const VkDevice& device,
                                         const Render& render) const {
  for (auto i = 0; i < render.GetMaxFramesInFlight() * UniformBufferNum; i++) {
    vkDestroyBuffer(device, uniformBuffers[i], nullptr);
    vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
  }
}

const MeshData* UniformBuffer::GetBridgeData() {
  if (bridge == nullptr) {
    bridge = dynamic_cast<Descriptor*>(owner)->GetBridgeData();
  }
  return bridge;
}

const MeshData* Descriptor::GetBridgeData() {
  if (bridge == nullptr) {
    bridge = dynamic_cast<Mesh*>(owner)->GetBridgeData();
  }
  return bridge;
}

#undef DoSomethingWithBuffers
#undef CopyMemoryOfBuffers
#undef CreateAndMapBuffers
#undef AddDescriptorWrites