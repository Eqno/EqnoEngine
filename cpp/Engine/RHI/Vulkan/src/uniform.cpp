#include "../include/uniform.h"

#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Light/include/BaseLight.h>
#include <Engine/Light/include/LightChannel.h>
#include <Engine/Model/include/BaseMaterial.h>

#include <chrono>
#include <ranges>
#include <stdexcept>

#include "../include/buffer.h"
#include "../include/device.h"
#include "../include/mesh.h"
#include "../include/render.h"
#include "../include/texture.h"
#include "../include/uniform.h"

/////////////////////////// POOLS ///////////////////////////

BufferManager& Descriptor::GetBufferManager() const {
  return static_cast<Mesh*>(owner)->GetBufferManager();
}

void Descriptor::CreateColorDescriptorPool(const VkDevice& device,
                                           Render& render,
                                           const size_t textureNum) {
  if (render.GetEnableDeferred()) {
    // Deferred shading output gBuffer
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (uint32_t i = 0; i < UniformBufferNum - 1; i++) {
      poolSizes.push_back({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                           .descriptorCount = static_cast<uint32_t>(
                               render.GetMaxFramesInFlight())});
    }
    for (uint32_t i = 0; i < textureNum; i++) {
      poolSizes.push_back({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                           .descriptorCount = static_cast<uint32_t>(
                               render.GetMaxFramesInFlight())});
    }
    VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = static_cast<uint32_t>(render.GetMaxFramesInFlight()),
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr,
                               &colorDescriptorPool) != VK_SUCCESS) {
      throw std::runtime_error("failed to create descriptor pool!");
    }

    // Deferred shading process gBuffer -> draw.cpp
  } else {
    // Forward shading
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (uint32_t i = 0; i < UniformBufferNum; i++) {
      poolSizes.push_back({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                           .descriptorCount = static_cast<uint32_t>(
                               render.GetMaxFramesInFlight())});
    }
    if (render.GetEnableShadowMap()) {
      poolSizes.push_back(
          {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
           .descriptorCount = static_cast<uint32_t>(
               render.GetMaxFramesInFlight() * render.GetShadowMapDepthNum())});
    }
    for (uint32_t i = 0; i < textureNum; i++) {
      poolSizes.push_back({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                           .descriptorCount = static_cast<uint32_t>(
                               render.GetMaxFramesInFlight())});
    }
    const VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = static_cast<uint32_t>(render.GetMaxFramesInFlight()),
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr,
                               &colorDescriptorPool) != VK_SUCCESS) {
      throw std::runtime_error("failed to create descriptor pool!");
    }
  }
}

void Descriptor::CreateZPrePassDescriptorPool(const VkDevice& device,
                                              const Render& render) {
  VkDescriptorPoolSize poolSize{
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = static_cast<uint32_t>(render.GetMaxFramesInFlight())};

  const VkDescriptorPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = static_cast<uint32_t>(render.GetMaxFramesInFlight()),
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize,
  };
  if (vkCreateDescriptorPool(device, &poolInfo, nullptr,
                             &zPrePassDescriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

void Descriptor::CreateShadowMapDescriptorPool(
    const VkDevice& device, const Render& render,
    VkDescriptorPool& descriptorPool) {
  VkDescriptorPoolSize poolSize{
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = static_cast<uint32_t>(render.GetMaxFramesInFlight())};

  const VkDescriptorPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = static_cast<uint32_t>(render.GetMaxFramesInFlight()),
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize,
  };
  if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

/////////////////////////// DSETS ///////////////////////////

#define AddDescriptorWrite(member, type, buf)                         \
  {                                                                   \
    VkDescriptorBufferInfo bufferInfo{                                \
        .buffer = (buf)->GetUniformBufferByIndex(i),                  \
        .offset = 0,                                                  \
        .range = sizeof(type),                                        \
    };                                                                \
    descriptorWrites.push_back({                                      \
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,              \
        .dstSet = member##DescriptorSets[i],                          \
        .dstBinding = static_cast<uint32_t>(descriptorWrites.size()), \
        .dstArrayElement = 0,                                         \
        .descriptorCount = 1,                                         \
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          \
        .pBufferInfo = &bufferInfo,                                   \
    });                                                               \
  }

void Descriptor::CreateColorDescriptorSets(
    const VkDevice& device, Render& render,
    const VkDescriptorSetLayout& descriptorSetLayout,
    const std::vector<Texture>& textures) {
  const std::vector layouts(render.GetMaxFramesInFlight(), descriptorSetLayout);

  if (render.GetEnableDeferred()) {
    // Deferred shading output gBuffer
    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = colorDescriptorPool,
        .descriptorSetCount =
            static_cast<uint32_t>(render.GetMaxFramesInFlight()),
        .pSetLayouts = layouts.data(),
    };

    colorDescriptorSets.resize(render.GetMaxFramesInFlight());
    if (vkAllocateDescriptorSets(device, &allocInfo,
                                 colorDescriptorSets.data()) != VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (auto i = 0; i < render.GetMaxFramesInFlight(); i++) {
      std::vector<VkWriteDescriptorSet> descriptorWrites;

      AddDescriptorWrite(color, CameraData, cameraBuffer);
      AddDescriptorWrite(color, MaterialData, materialBuffer);
      AddDescriptorWrite(color, TransformData, &transformBuffer);

      //  Do not merge the loops because `push_back` would move the memory to
      //  another location
      std::vector<VkDescriptorImageInfo> imageInfos;
      for (size_t j = 0; j < textures.size(); j++) {
        imageInfos.push_back({
            .sampler = textures[j].GetTextureSampler(),
            .imageView = textures[j].GetTextureImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
      }
      for (size_t j = 0; j < textures.size(); j++) {
        descriptorWrites.push_back({
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = colorDescriptorSets[i],
            .dstBinding = static_cast<uint32_t>(descriptorWrites.size()),
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfos[j],
        });
      }
      vkUpdateDescriptorSets(device,
                             static_cast<uint32_t>(descriptorWrites.size()),
                             descriptorWrites.data(), 0, nullptr);
    }

    // Deferred shading process gBuffer -> draw.cpp
  } else {
    // Forward shading
    const VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = colorDescriptorPool,
        .descriptorSetCount =
            static_cast<uint32_t>(render.GetMaxFramesInFlight()),
        .pSetLayouts = layouts.data(),
    };

    colorDescriptorSets.resize(render.GetMaxFramesInFlight());
    if (vkAllocateDescriptorSets(device, &allocInfo,
                                 colorDescriptorSets.data()) != VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (auto i = 0; i < render.GetMaxFramesInFlight(); i++) {
      std::vector<VkWriteDescriptorSet> descriptorWrites;

      AddDescriptorWrite(color, CameraData, cameraBuffer);
      AddDescriptorWrite(color, MaterialData, materialBuffer);
      AddDescriptorWrite(color, TransformData, &transformBuffer);
      AddDescriptorWrite(color, LightChannelData, lightChannelBuffer);

      // Put the codes outside if enable shadow map or it will be destructed
      uint32_t shadowMapDepthNum = render.GetShadowMapDepthNum();
      std::vector<VkDescriptorImageInfo> shadowMapImageInfos;

      if (render.GetEnableShadowMap()) {
        for (uint32_t j = 0; j < shadowMapDepthNum; j++) {
          shadowMapImageInfos.push_back({
              .sampler = render.GetShadowMapDepthSamplerByIndex(j),
              .imageView = render.GetShadowMapDepthImageViewByIndex(j),
              .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
          });
        }
        descriptorWrites.push_back({
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = colorDescriptorSets[i],
            .dstBinding = static_cast<uint32_t>(descriptorWrites.size()),
            .dstArrayElement = 0,
            .descriptorCount = shadowMapDepthNum,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = shadowMapImageInfos.data(),
        });
      }

      //  Do not merge the loops because `push_back` would move the memory to
      //  another location
      std::vector<VkDescriptorImageInfo> imageInfos;
      for (size_t j = 0; j < textures.size(); j++) {
        imageInfos.push_back({
            .sampler = textures[j].GetTextureSampler(),
            .imageView = textures[j].GetTextureImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
      }
      for (size_t j = 0; j < textures.size(); j++) {
        descriptorWrites.push_back({
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = colorDescriptorSets[i],
            .dstBinding = static_cast<uint32_t>(descriptorWrites.size()),
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfos[j],
        });
      }
      vkUpdateDescriptorSets(device,
                             static_cast<uint32_t>(descriptorWrites.size()),
                             descriptorWrites.data(), 0, nullptr);
    }
  }
}

void Descriptor::CreateZPrePassDescriptorSets(
    const VkDevice& device, const Render& render,
    const VkDescriptorSetLayout& descriptorSetLayout) {
  const std::vector layouts(render.GetMaxFramesInFlight(), descriptorSetLayout);

  const VkDescriptorSetAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = zPrePassDescriptorPool,
      .descriptorSetCount =
          static_cast<uint32_t>(render.GetMaxFramesInFlight()),
      .pSetLayouts = layouts.data(),
  };

  zPrePassDescriptorSets.resize(render.GetMaxFramesInFlight());
  if (vkAllocateDescriptorSets(device, &allocInfo,
                               zPrePassDescriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor sets!");
  }

  for (auto i = 0; i < render.GetMaxFramesInFlight(); i++) {
    std::vector<VkWriteDescriptorSet> descriptorWrites;
    AddDescriptorWrite(zPrePass, TransformData, &transformBuffer);

    vkUpdateDescriptorSets(device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void Descriptor::CreateShadowMapDescriptorSets(
    const VkDevice& device, const Render& render,
    const VkDescriptorSetLayout& descriptorSetLayout,
    const VkDescriptorPool& descriptorPool,
    const ShadowMapBuffer& shadowMapBuffer, DescriptorSets& descriptorSets) {
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
    VkDescriptorBufferInfo bufferInfo{
        .buffer = shadowMapBuffer.GetUniformBufferByIndex(i),
        .offset = 0,
        .range = sizeof(TransformData),
    };
    VkWriteDescriptorSet descriptorWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &bufferInfo,
    };
    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
  }
}

/////////////////////////// UPDATE ///////////////////////////

void Descriptor::UpdateBufferPointers() {
  if (auto bridgePtr = GetBridgeData().lock()) {
    const UniformData& uniform = bridgePtr->uniform;
    if (auto camera = uniform.camera.lock()) {
      cameraPointer = camera.get();
    }
    if (auto material = uniform.material.lock()) {
      materialPointer = material.get();
    }
    if (auto lightChannel = uniform.lightChannel.lock()) {
      lightChannelPointer = lightChannel.get();
    }
  }
}

void Descriptor::UpdateColorDescriptorSets(const VkDevice& device,
                                           Render& render) {
  for (auto i = 0; i < render.GetMaxFramesInFlight(); i++) {
    VkWriteDescriptorSet descriptorWrite;

    uint32_t shadowMapDepthNum = render.GetShadowMapDepthNum();
    std::vector<VkDescriptorImageInfo> shadowMapImageInfos;

    for (size_t j = 0; j < shadowMapDepthNum; j++) {
      shadowMapImageInfos.push_back({
          .sampler = render.GetShadowMapDepthSamplerByIndex(j),
          .imageView = render.GetShadowMapDepthImageViewByIndex(j),
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      });
    }
    descriptorWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = colorDescriptorSets[i],
        .dstBinding = static_cast<uint32_t>(UniformBufferNum),
        .dstArrayElement = 0,
        .descriptorCount = shadowMapDepthNum,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = shadowMapImageInfos.data(),
    };
    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
  }
}

void Descriptor::UpdateUniformBuffer(const uint32_t currentImage) {
  transformBuffer.UpdateUniformBuffer(currentImage);

  UpdateBufferPointers();
  if (cameraPointer != nullptr) {
    cameraBuffer->UpdateUniformBuffer(cameraPointer, currentImage);
  }
  if (materialPointer != nullptr) {
    materialBuffer->UpdateUniformBuffer(materialPointer, currentImage);
  }
  if (lightChannelPointer != nullptr) {
    lightChannelBuffer->UpdateUniformBuffer(lightChannelPointer, currentImage);
  }
}

void TransformBuffer::UpdateUniformBuffer(const uint32_t currentImage) {
  if (auto bridgePtr = GetBridgeData().lock()) {
    if (auto camera = bridgePtr->uniform.camera.lock()) {
      TransformData* buffer =
          reinterpret_cast<TransformData*>(uniformBuffersMapped[currentImage]);

      glm::mat4x4* modelMatrix = bridgePtr->uniform.modelMatrix;
      buffer->modelMatrix = modelMatrix ? *modelMatrix : Mat4x4Zero;

      camera->GetMatrixLock().lock();
      buffer->viewMatrix = camera->GetViewMatrix();
      buffer->projMatrix = camera->GetProjMatrix();
      camera->GetMatrixLock().unlock();
    }
  }
}

void ShadowMapBuffer::UpdateUniformBuffer(const uint32_t currentImage,
                                          BaseLight* shadowMapLight) {
  if (auto bridgePtr = GetBridgeData().lock()) {
    TransformData* buffer =
        reinterpret_cast<TransformData*>(uniformBuffersMapped[currentImage]);

    glm::mat4x4* modelMatrix = bridgePtr->uniform.modelMatrix;
    buffer->modelMatrix = modelMatrix ? *modelMatrix : Mat4x4Zero;

    shadowMapLight->GetMatrixLock().lock();
    buffer->viewMatrix = shadowMapLight->GetViewMatrix();
    buffer->projMatrix = shadowMapLight->GetProjMatrix();
    shadowMapLight->GetMatrixLock().unlock();
  }
}

void Descriptor::UpdateShadowMapUniformBuffers(
    const Device& device, const Render& render, const uint32_t currentImage,
    std::unordered_map<int, std::weak_ptr<BaseLight>>& lightsById) {
  auto iter = lightsById.begin();
  while (iter != lightsById.end()) {
    if (auto lightPtr = iter->second.lock()) {
      CreateShadowMapUniformBufferByIndex(device, render, lightPtr->GetId())
          .UpdateUniformBuffer(currentImage, lightPtr.get());
      CreateShadowMapDescriptorSetByIndices(device, render, lightPtr->GetId(),
                                            currentImage);
      iter++;
    } else {
      RemoveShadowMapUniformBufferByIndex(device.GetLogical(), render,
                                          iter->first);
      RemoveShadowMapDescriptorSetsByIndex(device.GetLogical(), iter->first);
      iter = lightsById.erase(iter);
    }
  }
}

/////////////////////////// BUFFER ///////////////////////////

void Descriptor::CreateUniformBuffer(const Device& device, Render& render) {
  transformBuffer.CreateUniformBuffer(device, render, sizeof(TransformData));

  UpdateBufferPointers();
  if (cameraPointer != nullptr) {
    cameraBuffer =
        &GetBufferManager().CreateCameraBuffer(cameraPointer, device, render);
  }
  if (materialPointer != nullptr) {
    materialBuffer = &GetBufferManager().CreateMaterialBuffer(materialPointer,
                                                              device, render);
  }
  if (lightChannelPointer != nullptr) {
    lightChannelBuffer = &GetBufferManager().CreateLightChannelBuffer(
        lightChannelPointer, device, render);
  }
}

void Descriptor::DestroyUniformBuffer(const VkDevice& device,
                                      const Render& render) {
  transformBuffer.DestroyUniformBuffer(device, render);

  if (render.GetEnableShadowMap()) {
    for (const ShadowMapBuffer& shadowMapBuffer :
         shadowMapBuffers | std::views::values) {
      shadowMapBuffer.DestroyUniformBuffer(device, render);
    }
  }

  GetBufferManager().DestroyCameraBuffer(cameraPointer, device, render);
  GetBufferManager().DestroyMaterialBuffer(materialPointer, device, render);
  GetBufferManager().DestroyLightChannelBuffer(lightChannelPointer, device,
                                               render);
}

/////////////////////////// DESCRIPTOR ///////////////////////////

void Descriptor::CreateDescriptor(
    const Device& device, Render& render, const std::vector<Texture>& textures,
    const VkDescriptorSetLayout& colorDescriptorSetLayout,
    const VkDescriptorSetLayout& zPrePassDescriptorSetLayout,
    const VkDescriptorSetLayout& shadowMapDescriptorSetLayout) {
  CreateUniformBuffer(device, render);
  CreateColorDescriptorPool(device.GetLogical(), render, textures.size());
  CreateColorDescriptorSets(device.GetLogical(), render,
                            colorDescriptorSetLayout, textures);

  if (render.GetEnableZPrePass()) {
    CreateZPrePassDescriptorPool(device.GetLogical(), render);
    CreateZPrePassDescriptorSets(device.GetLogical(), render,
                                 zPrePassDescriptorSetLayout);
  }

  if (render.GetEnableShadowMap()) {
    this->shadowMapDescriptorSetLayout = shadowMapDescriptorSetLayout;
  }
}

void Descriptor::DestroyDesciptor(const VkDevice& device,
                                  const Render& render) {
  vkDestroyDescriptorPool(device, colorDescriptorPool, nullptr);
  if (render.GetEnableZPrePass()) {
    vkDestroyDescriptorPool(device, zPrePassDescriptorPool, nullptr);
  }
  if (render.GetEnableShadowMap()) {
    for (const VkDescriptorPool& descriptorPool :
         shadowMapDescriptorPools | std::views::values) {
      vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }
  }
  DestroyUniformBuffer(device, render);
}

std::weak_ptr<MeshData> ShadowMapBuffer::GetBridgeData() {
  return static_cast<Descriptor*>(owner)->GetBridgeData();
}

std::weak_ptr<MeshData> TransformBuffer::GetBridgeData() {
  return static_cast<Descriptor*>(owner)->GetBridgeData();
}

std::weak_ptr<MeshData> Descriptor::GetBridgeData() {
  return static_cast<Mesh*>(owner)->GetBridgeData();
}

ShadowMapBuffer& Descriptor::CreateShadowMapUniformBufferByIndex(
    const Device& device, const Render& render, const uint32_t lightId) {
  if (shadowMapBuffers.contains(lightId) == false) {
    RegisterMember(shadowMapBuffers[lightId]);
    shadowMapBuffers[lightId].CreateUniformBuffer(device, render,
                                                  sizeof(TransformData));
  }
  return shadowMapBuffers[lightId];
}

void Descriptor::RemoveShadowMapUniformBufferByIndex(const VkDevice& device,
                                                     const Render& render,
                                                     const uint32_t lightId) {
  if (shadowMapBuffers.contains(lightId)) {
    shadowMapBuffers[lightId].DestroyUniformBuffer(device, render);
    shadowMapBuffers.erase(lightId);
  }
}

const VkDescriptorPool& Descriptor::CreateShadowMapDescriptorPoolByIndex(
    const VkDevice& device, const Render& render, const uint32_t lightId) {
  if (shadowMapDescriptorPools.contains(lightId) == false) {
    CreateShadowMapDescriptorPool(device, render,
                                  shadowMapDescriptorPools[lightId]);
  }
  return shadowMapDescriptorPools[lightId];
}

void Descriptor::RemoveShadowMapDescriptorPoolByIndex(const VkDevice& device,
                                                      const uint32_t lightId) {
  if (shadowMapDescriptorPools.contains(lightId)) {
    vkDestroyDescriptorPool(device, shadowMapDescriptorPools[lightId], nullptr);
    shadowMapDescriptorPools.erase(lightId);
  }
}

const DescriptorSets& Descriptor::CreateShadowMapDescriptorSetsByIndex(
    const Device& device, const Render& render, const uint32_t lightId) {
  if (shadowMapDescriptorSets.contains(lightId) == false) {
    CreateShadowMapDescriptorSets(
        device.GetLogical(), render, shadowMapDescriptorSetLayout,
        CreateShadowMapDescriptorPoolByIndex(device.GetLogical(), render,
                                             lightId),
        CreateShadowMapUniformBufferByIndex(device, render, lightId),
        shadowMapDescriptorSets[lightId]);
  }
  return shadowMapDescriptorSets[lightId];
}

const VkDescriptorSet& Descriptor::CreateShadowMapDescriptorSetByIndices(
    const Device& device, const Render& render, const uint32_t lightId,
    const uint32_t currentFrame) {
  return CreateShadowMapDescriptorSetsByIndex(device, render,
                                              lightId)[currentFrame];
}

void Descriptor::RemoveShadowMapDescriptorSetsByIndex(const VkDevice& device,
                                                      const uint32_t lightId) {
  if (shadowMapDescriptorSets.contains(lightId)) {
    RemoveShadowMapDescriptorPoolByIndex(device, lightId);
    shadowMapDescriptorSets.erase(lightId);
  }
}