#include "../include/uniform.h"

#include <chrono>
#include <stdexcept>

#include "../include/buffer.h"
#include "../include/device.h"
#include "../include/mesh.h"
#include "../include/render.h"
#include "../include/texture.h"

#define UniformBufferNum 2
#define TransformBufferIndex(i) (i * UniformBufferNum)
#define MaterialBufferIndex(i) (i * UniformBufferNum + 1)

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
    descriptorWrites[0] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
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
    descriptorWrites[1] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &materialBufferInfo,
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
  poolSizes.resize(textureNum + 2);

  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount =
      static_cast<uint32_t>(render.GetMaxFramesInFlight());

  poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[1].descriptorCount =
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

    vkMapMemory(device.GetLogical(),
                uniformBuffersMemory[TransformBufferIndex(i)], 0,
                sizeof(TransformBufferObject), 0,
                &uniformBuffersMapped[TransformBufferIndex(i)]);
    vkMapMemory(device.GetLogical(),
                uniformBuffersMemory[MaterialBufferIndex(i)], 0,
                sizeof(MaterialBufferObject), 0,
                &uniformBuffersMapped[MaterialBufferIndex(i)]);
  }
}

void UniformBuffer::UpdateUniformBuffer(const uint32_t currentImage) const {
  TransformBufferObject transform{
      .model = *dynamic_cast<Descriptor*>(owner)->GetModelMatrix(),
      .view = *dynamic_cast<Descriptor*>(owner)->GetViewMatrix(),
      .proj = *dynamic_cast<Descriptor*>(owner)->GetProjMatrix(),
  };
  transform.proj[1][1] *= -1;
  MaterialBufferObject material{
      .color = *dynamic_cast<Descriptor*>(owner)->GetBaseColor(),
      .roughness = dynamic_cast<Descriptor*>(owner)->GetRoughness(),
      .metallic = dynamic_cast<Descriptor*>(owner)->GetMetallic(),
  };
  memcpy(uniformBuffersMapped[TransformBufferIndex(currentImage)], &transform,
         sizeof(transform));
  memcpy(uniformBuffersMapped[MaterialBufferIndex(currentImage)], &material,
         sizeof(material));
}

void UniformBuffer::DestroyUniformBuffer(const VkDevice& device,
                                         const Render& render) const {
  for (auto i = 0; i < render.GetMaxFramesInFlight() * UniformBufferNum; i++) {
    vkDestroyBuffer(device, uniformBuffers[i], nullptr);
    vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
  }
}

// Update uniform buffer data
const glm::mat4x4* Descriptor::GetModelMatrix() {
  return dynamic_cast<Mesh*>(owner)->GetModelMatrix();
}
const glm::mat4x4* Descriptor::GetViewMatrix() {
  return dynamic_cast<Mesh*>(owner)->GetViewMatrix();
}
const glm::mat4x4* Descriptor::GetProjMatrix() {
  return dynamic_cast<Mesh*>(owner)->GetProjMatrix();
}
const glm::vec4* Descriptor::GetBaseColor() {
  return dynamic_cast<Mesh*>(owner)->GetBaseColor();
}
const float Descriptor::GetRoughness() {
  return dynamic_cast<Mesh*>(owner)->GetRoughness();
}
const float Descriptor::GetMetallic() {
  return dynamic_cast<Mesh*>(owner)->GetMetallic();
}

#undef UniformBufferNum
#undef TransformBufferIndex
#undef MaterialBufferIndex