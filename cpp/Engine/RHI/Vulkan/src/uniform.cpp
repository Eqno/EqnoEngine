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

void Descriptor::CreateColorDescriptorPool(const VkDevice& device,
                                           Render& render,
                                           const size_t textureNum) {
  std::vector<VkDescriptorPoolSize> poolSizes(UniformBufferNum + 1 +
                                              textureNum);
  for (int i = 0; i < UniformBufferNum; i++) {
    poolSizes[i].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[i].descriptorCount =
        static_cast<uint32_t>(render.GetMaxFramesInFlight());
  }
  poolSizes[UniformBufferNum].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[UniformBufferNum].descriptorCount = static_cast<uint32_t>(
      render.GetMaxFramesInFlight() * render.GetShadowMapDepthNum());
  for (size_t i = 1; i < 1 + textureNum; i++) {
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
  if (vkCreateDescriptorPool(device, &poolInfo, nullptr,
                             &colorDescriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
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

#define AddDescriptorWrite(member, index, type, buf)         \
  {                                                          \
    VkDescriptorBufferInfo bufferInfo{                       \
        .buffer = (buf)->GetUniformBufferByIndex(i),         \
        .offset = 0,                                         \
        .range = sizeof(type),                               \
    };                                                       \
    descriptorWrites[index] = {                              \
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,     \
        .dstSet = member##DescriptorSets[i],                 \
        .dstBinding = index,                                 \
        .dstArrayElement = 0,                                \
        .descriptorCount = 1,                                \
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, \
        .pBufferInfo = &bufferInfo,                          \
    };                                                       \
  }
#define AddColorDescriptorWrites()                                      \
  {                                                                     \
    AddDescriptorWrite(color, 0, CameraData, cameraBuffer);             \
    AddDescriptorWrite(color, 1, MaterialData, materialBuffer);         \
    AddDescriptorWrite(color, 2, TransformData, &transformBuffer);      \
    AddDescriptorWrite(color, 3, LightChannelData, lightChannelBuffer); \
  }
#define AddZPrePassDescriptorWrites() \
  AddDescriptorWrite(zPrePass, 0, TransformData, &transformBuffer)

void Descriptor::CreateColorDescriptorSets(
    const VkDevice& device, Render& render,
    const VkDescriptorSetLayout& descriptorSetLayout,
    const std::vector<Texture>& textures) {
  const std::vector layouts(render.GetMaxFramesInFlight(), descriptorSetLayout);

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
    std::vector<VkWriteDescriptorSet> descriptorWrites(UniformBufferNum + 1 +
                                                       textures.size());
    AddColorDescriptorWrites();

    uint32_t shadowMapDepthNum = render.GetShadowMapDepthNum();
    std::vector<VkDescriptorImageInfo> shadowMapImageInfos(shadowMapDepthNum);

    for (uint32_t j = 0; j < shadowMapDepthNum; j++) {
      shadowMapImageInfos[j] = {
          .sampler = render.GetShadowMapDepthSamplerByIndex(j),
          .imageView = render.GetShadowMapDepthImageViewByIndex(j),
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
    }
    descriptorWrites[UniformBufferNum] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = colorDescriptorSets[i],
        .dstBinding = static_cast<uint32_t>(UniformBufferNum),
        .dstArrayElement = 0,
        .descriptorCount = shadowMapDepthNum,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = shadowMapImageInfos.data(),
    };

    std::vector<VkDescriptorImageInfo> imageInfos(textures.size());
    for (size_t j = 0; j < textures.size(); j++) {
      imageInfos[j] = {
          .sampler = textures[j].GetTextureSampler(),
          .imageView = textures[j].GetTextureImageView(),
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
      descriptorWrites[UniformBufferNum + 1 + j] = {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = colorDescriptorSets[i],
          .dstBinding = static_cast<uint32_t>(UniformBufferNum + 1 + j),
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
    std::vector<VkWriteDescriptorSet> descriptorWrites(1);
    AddZPrePassDescriptorWrites();

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
    bufferManager = uniform.bufferManager;
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
    std::vector<VkDescriptorImageInfo> shadowMapImageInfos(shadowMapDepthNum);

    for (size_t j = 0; j < shadowMapDepthNum; j++) {
      shadowMapImageInfos[j] = {
          .sampler = render.GetShadowMapDepthSamplerByIndex(j),
          .imageView = render.GetShadowMapDepthImageViewByIndex(j),
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
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
  if (bufferManager == nullptr) {
    return;
  }
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
  if (bufferManager == nullptr) {
    return;
  }
  if (cameraPointer != nullptr) {
    cameraBuffer =
        &bufferManager->CreateCameraBuffer(cameraPointer, device, render);
  }
  if (materialPointer != nullptr) {
    materialBuffer =
        &bufferManager->CreateMaterialBuffer(materialPointer, device, render);
  }
  if (lightChannelPointer != nullptr) {
    lightChannelBuffer = &bufferManager->CreateLightChannelBuffer(
        lightChannelPointer, device, render);
  }
}

void Descriptor::DestroyUniformBuffer(const VkDevice& device,
                                      const Render& render) {
  transformBuffer.DestroyUniformBuffer(device, render);
  for (const ShadowMapBuffer& shadowMapBuffer :
       shadowMapBuffers | std::views::values) {
    shadowMapBuffer.DestroyUniformBuffer(device, render);
  }

  if (bufferManager == nullptr) {
    return;
  }
  bufferManager->DestroyCameraBuffer(cameraPointer, device, render);
  bufferManager->DestroyMaterialBuffer(materialPointer, device, render);
  bufferManager->DestroyLightChannelBuffer(lightChannelPointer, device, render);
}

/////////////////////////// DESCRIPTOR ///////////////////////////

void Descriptor::CreateDescriptor(
    const Device& device, Render& render, const std::vector<Texture>& textures,
    const VkDescriptorSetLayout& colorDescriptorSetLayout,
    const VkDescriptorSetLayout& zPrePassDescriptorSetLayout,
    const VkDescriptorSetLayout& shadowMapDescriptorSetLayout) {
  CreateUniformBuffer(device, render);

  CreateColorDescriptorPool(device.GetLogical(), render, textures.size());
  if (render.GetEnableZPrePass()) {
    CreateZPrePassDescriptorPool(device.GetLogical(), render);
  }

  CreateColorDescriptorSets(device.GetLogical(), render,
                            colorDescriptorSetLayout, textures);
  if (render.GetEnableZPrePass()) {
    CreateZPrePassDescriptorSets(device.GetLogical(), render,
                                 zPrePassDescriptorSetLayout);
  }

  this->shadowMapDescriptorSetLayout = shadowMapDescriptorSetLayout;
}

void Descriptor::DestroyDesciptor(const VkDevice& device,
                                  const Render& render) {
  vkDestroyDescriptorPool(device, colorDescriptorPool, nullptr);
  if (render.GetEnableZPrePass()) {
    vkDestroyDescriptorPool(device, zPrePassDescriptorPool, nullptr);
  }
  for (const VkDescriptorPool& descriptorPool :
       shadowMapDescriptorPools | std::views::values) {
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
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

#undef AddColorDescriptorWrites
#undef AddZPrePassDescriptorWrites

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