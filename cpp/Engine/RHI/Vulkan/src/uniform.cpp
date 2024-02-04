#include "../include/uniform.h"

#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Light/include/BaseLight.h>
#include <Engine/Light/include/LightChannel.h>
#include <Engine/Model/include/BaseMaterial.h>

#include <chrono>
#include <stdexcept>

#include "../include/buffer.h"
#include "../include/device.h"
#include "../include/mesh.h"
#include "../include/render.h"
#include "../include/texture.h"
#include "../include/uniform.h"

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

    {
      VkDescriptorBufferInfo bufferInfo{
          .buffer = cameraBuffer->GetUniformBufferByIndex(i),
          .offset = 0,
          .range = sizeof(CameraData),
      };
      descriptorWrites[0] = {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = descriptorSets[i],
          .dstBinding = 0,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .pBufferInfo = &bufferInfo,
      };
    }
    {
      VkDescriptorBufferInfo bufferInfo{
          .buffer = materialBuffer->GetUniformBufferByIndex(i),
          .offset = 0,
          .range = sizeof(MaterialData),
      };
      descriptorWrites[1] = {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = descriptorSets[i],
          .dstBinding = 1,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .pBufferInfo = &bufferInfo,
      };
    }
    {
      VkDescriptorBufferInfo bufferInfo{
          .buffer = transformBuffer.GetUniformBufferByIndex(i),
          .offset = 0,
          .range = sizeof(TransformData),
      };
      descriptorWrites[2] = {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = descriptorSets[i],
          .dstBinding = 2,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .pBufferInfo = &bufferInfo,
      };
    }
    {
      VkDescriptorBufferInfo bufferInfo{
          .buffer = lightChannelBuffer->GetUniformBufferByIndex(i),
          .offset = 0,
          .range = sizeof(LightsData),
      };
      descriptorWrites[3] = {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = descriptorSets[i],
          .dstBinding = 3,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .pBufferInfo = &bufferInfo,
      };
    }

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
  transformBuffer.CreateUniformBuffer(device, render, sizeof(TransformData));

  const UniformData& uniform = GetBridgeData()->uniform;
  if (BaseCamera* camera = uniform.camera) {
    cameraBuffer =
        &uniform.bufferManager->CreateCameraBuffer(camera, device, render);
  }
  if (BaseMaterial* material = uniform.material) {
    materialBuffer =
        &uniform.bufferManager->CreateMaterialBuffer(material, device, render);
  }
  if (LightChannel* lightChannel = uniform.lights) {
    lightChannelBuffer = &uniform.bufferManager->CreateLightChannelBuffer(
        lightChannel, device, render);
  }
}
void Descriptor::UpdateUniformBuffer(const uint32_t currentImage) {
  transformBuffer.UpdateUniformBuffer(currentImage);

  const UniformData& uniform = GetBridgeData()->uniform;
  if (BaseCamera* camera = uniform.camera) {
    cameraBuffer->UpdateUniformBuffer(camera, currentImage);
  }
  if (BaseMaterial* material = uniform.material) {
    materialBuffer->UpdateUniformBuffer(material, currentImage);
  }
  if (LightChannel* lightChannel = uniform.lights) {
    lightChannelBuffer->UpdateUniformBuffer(lightChannel, currentImage);
  }
}
void Descriptor::DestroyUniformBuffer(const VkDevice& device,
                                      const Render& render) {
  transformBuffer.DestroyUniformBuffer(device, render);

  const UniformData& uniform = GetBridgeData()->uniform;
  if (BaseCamera* camera = uniform.camera) {
    uniform.bufferManager->DestroyCameraBuffer(camera, device, render);
  }
  if (BaseMaterial* material = uniform.material) {
    uniform.bufferManager->DestroyMaterialBuffer(material, device, render);
  }
  if (LightChannel* lightChannel = uniform.lights) {
    uniform.bufferManager->DestroyLightChannelBuffer(lightChannel, device,
                                                     render);
  }
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
                                  const Render& render) {
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  DestroyUniformBuffer(device, render);
}

void TransformBuffer::UpdateUniformBuffer(const uint32_t currentImage) {
  if (BaseCamera* camera = GetBridgeData()->uniform.camera) {
    TransformData* buffer =
        reinterpret_cast<TransformData*>(uniformBuffersMapped[currentImage]);
    glm::mat4x4* modelMatrix = GetBridgeData()->uniform.modelMatrix;
    buffer->modelMatrix = modelMatrix ? *modelMatrix : Mat4x4Zero;
    buffer->viewMatrix = camera->GetViewMatrix();
    buffer->projMatrix = camera->GetProjMatrix();
  }
}

const MeshData* TransformBuffer::GetBridgeData() {
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