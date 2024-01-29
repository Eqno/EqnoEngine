#include "../include/uniform.h"

#include <chrono>
#include <stdexcept>

#include "../include/buffer.h"
#include "../include/device.h"
#include "../include/mesh.h"
#include "../include/render.h"
#include "../include/texture.h"
#include "../include/uniform.h"

#define TransformDesctiptorIndex 0
#define MaterialDesctiptorIndex 1
#define LightDescriptorIndex 2

#define TransformBufferIndex(i) \
  (i * UniformBufferNum + TransformDesctiptorIndex)
#define MaterialBufferIndex(i) (i * UniformBufferNum + MaterialDesctiptorIndex)
#define LightBufferIndex(i) (i * UniformBufferNum + LightDescriptorIndex)

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

    VkDescriptorBufferInfo transformBufferInfo{
        .buffer = GetUniformBufferByIndex(TransformBufferIndex(i)),
        .offset = 0,
        .range = sizeof(TransformBufferObject),
    };
    descriptorWrites[TransformDesctiptorIndex] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = TransformDesctiptorIndex,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &transformBufferInfo,
    };

    VkDescriptorBufferInfo materialBufferInfo{
        .buffer = GetUniformBufferByIndex(MaterialBufferIndex(i)),
        .offset = 0,
        .range = sizeof(MaterialBufferObject),
    };
    descriptorWrites[MaterialDesctiptorIndex] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = MaterialDesctiptorIndex,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &materialBufferInfo,
    };

    VkDescriptorBufferInfo lightBufferInfo{
        .buffer = GetUniformBufferByIndex(LightBufferIndex(i)),
        .offset = 0,
        .range = sizeof(LightBufferObject) * MaxLightNum,
    };
    descriptorWrites[LightDescriptorIndex] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = LightDescriptorIndex,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &lightBufferInfo,
    };

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

  poolSizes[TransformDesctiptorIndex].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[TransformDesctiptorIndex].descriptorCount =
      static_cast<uint32_t>(render.GetMaxFramesInFlight());

  poolSizes[MaterialDesctiptorIndex].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[MaterialDesctiptorIndex].descriptorCount =
      static_cast<uint32_t>(render.GetMaxFramesInFlight());

  poolSizes[LightDescriptorIndex].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[LightDescriptorIndex].descriptorCount =
      static_cast<uint32_t>(render.GetMaxFramesInFlight());

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
  uniformBuffer.RegisterOwner(this);
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
    Buffer::CreateBuffer(device, sizeof(TransformBufferObject),
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         uniformBuffers[TransformBufferIndex(i)],
                         uniformBuffersMemory[TransformBufferIndex(i)]);

    Buffer::CreateBuffer(device, sizeof(MaterialBufferObject),
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         uniformBuffers[MaterialBufferIndex(i)],
                         uniformBuffersMemory[MaterialBufferIndex(i)]);

    Buffer::CreateBuffer(device, sizeof(LightBufferObject) * MaxLightNum,
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         uniformBuffers[LightBufferIndex(i)],
                         uniformBuffersMemory[LightBufferIndex(i)]);

    vkMapMemory(device.GetLogical(),
                uniformBuffersMemory[TransformBufferIndex(i)], 0,
                sizeof(TransformBufferObject), 0,
                &uniformBuffersMapped[TransformBufferIndex(i)]);
    vkMapMemory(device.GetLogical(),
                uniformBuffersMemory[MaterialBufferIndex(i)], 0,
                sizeof(MaterialBufferObject), 0,
                &uniformBuffersMapped[MaterialBufferIndex(i)]);
    vkMapMemory(device.GetLogical(), uniformBuffersMemory[LightBufferIndex(i)],
                0, sizeof(LightBufferObject), 0,
                &uniformBuffersMapped[LightBufferIndex(i)]);
  }
}

void UniformBuffer::UpdateUniformBuffer(const uint32_t currentImage) {
  TransformBufferObject transform{
      .model = *GetBridgeData()->uniform.transform.modelMatrix,
      .view = *GetBridgeData()->uniform.transform.viewMatrix,
      .proj = *GetBridgeData()->uniform.transform.projMatrix,
  };
  transform.proj[1][1] *= -1;
  MaterialBufferObject material{
      .color = *GetBridgeData()->uniform.material.color,
      .roughness = *GetBridgeData()->uniform.material.roughness,
      .metallic = *GetBridgeData()->uniform.material.metallic,
  };
  std::vector<LightBufferObject> lights;
  for (const LightData& light : GetBridgeData()->uniform.lights) {
    lights.push_back({static_cast<unsigned int>(*light.type), *light.intensity,
                      *light.pos, *light.color, *light.normal});
  }
  memcpy(uniformBuffersMapped[TransformBufferIndex(currentImage)], &transform,
         sizeof(TransformBufferObject));
  memcpy(uniformBuffersMapped[MaterialBufferIndex(currentImage)], &material,
         sizeof(MaterialBufferObject));
  memcpy(uniformBuffersMapped[LightBufferIndex(currentImage)], lights.data(),
         sizeof(LightBufferObject) * lights.size());
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

#undef TransformDesctiptorIndex
#undef MaterialDesctiptorIndex
#undef LightDescriptorIndex

#undef TransformBufferIndex
#undef MaterialBufferIndex
#undef LightBufferIndex