#include <Engine/RHI/Vulkan/include/config.h>
#include <Engine/RHI/Vulkan/include/draw.h>
#include <Engine/RHI/Vulkan/include/vulkan.h>

#include <vector>

BufferManager& Draw::GetBufferManager() const {
  return static_cast<Vulkan*>(owner)->GetBufferManager();
}

void Draw::CreateDeferredDescriptorPool(const VkDevice& device,
                                        Render& render) {
  // Deferred shading process gBuffer
  std::vector<VkDescriptorPoolSize> poolSizes;
  for (int i = 0; i < 2; i++) {
    poolSizes.push_back({.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                         .descriptorCount = static_cast<uint32_t>(
                             render.GetMaxFramesInFlight())});
  }
  for (uint32_t i = 0; i < GBUFFER_SIZE; i++) {
    poolSizes.push_back({.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                         .descriptorCount = static_cast<uint32_t>(
                             render.GetMaxFramesInFlight())});
  }
  if (render.GetEnableShadowMap()) {
    poolSizes.push_back(
        {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = static_cast<uint32_t>(
             render.GetMaxFramesInFlight() * render.GetShadowMapDepthNum())});
  }
  VkDescriptorPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = static_cast<uint32_t>(render.GetMaxFramesInFlight()),
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.data(),
  };
  if (vkCreateDescriptorPool(device, &poolInfo, nullptr,
                             &deferredDescriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

void Draw::CreateDeferredDescriptorSets(const Device& device, Render& render) {
  // Deferred shading process gBuffer
  const std::vector layouts(render.GetMaxFramesInFlight(),
                            pipeline.GetDeferredDescriptorSetLayout());
  VkDescriptorSetAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = deferredDescriptorPool,
      .descriptorSetCount =
          static_cast<uint32_t>(render.GetMaxFramesInFlight()),
      .pSetLayouts = layouts.data(),
  };

  deferredDescriptorSets.resize(render.GetMaxFramesInFlight());
  if (vkAllocateDescriptorSets(device.GetLogical(), &allocInfo,
                               deferredDescriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor sets!");
  }

  for (auto i = 0; i < render.GetMaxFramesInFlight(); i++) {
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    VkDescriptorBufferInfo pipelineBufferInfo{
        .buffer = pipelineBuffer.GetUniformBufferByIndex(i),
        .offset = 0,
        .range = sizeof(PipelineData),
    };
    descriptorWrites.push_back({
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = deferredDescriptorSets[i],
        .dstBinding = static_cast<uint32_t>(descriptorWrites.size()),
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &pipelineBufferInfo,
    });

    VkDescriptorBufferInfo allLightsChannelBufferInfo{
        .buffer = allLightsChannelBufferPointer->GetUniformBufferByIndex(i),
        .offset = 0,
        .range = sizeof(LightChannelData),
    };
    descriptorWrites.push_back({
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = deferredDescriptorSets[i],
        .dstBinding = static_cast<uint32_t>(descriptorWrites.size()),
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &allLightsChannelBufferInfo,
    });

    //  Do not merge the loops because `push_back` would move the memory to
    //  another location
    std::vector<VkDescriptorImageInfo> gBufferImageInfos;
    for (uint32_t j = 0; j < GBUFFER_SIZE; j++) {
      gBufferImageInfos.push_back({
          .sampler = VK_NULL_HANDLE,
          .imageView = render.GetGBufferImageViewByIndex(j),
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      });
    }
    for (uint32_t j = 0; j < GBUFFER_SIZE; j++) {
      descriptorWrites.push_back({
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = deferredDescriptorSets[i],
          .dstBinding = static_cast<uint32_t>(descriptorWrites.size()),
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
          .pImageInfo = &gBufferImageInfos[j],
      });
    }

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
          .dstSet = deferredDescriptorSets[i],
          .dstBinding = static_cast<uint32_t>(descriptorWrites.size()),
          .dstArrayElement = 0,
          .descriptorCount = shadowMapDepthNum,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .pImageInfo = shadowMapImageInfos.data(),
      });
    }
    vkUpdateDescriptorSets(device.GetLogical(),
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void Draw::UpdateDeferredDescriptorSets(const VkDevice& device,
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
        .dstSet = deferredDescriptorSets[i],
        .dstBinding = 2 + GBUFFER_SIZE,
        .dstArrayElement = 0,
        .descriptorCount = shadowMapDepthNum,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = shadowMapImageInfos.data(),
    };
    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
  }
}

void Draw::CreateDrawResource(const Device& device, Render& render,
                              const std::string& rootPath, const int texCount,
                              const std::vector<std::string>& shaderPaths,
                              const std::string& zPrePassShaderPath,
                              const std::string& shadowMapShaderPath) {
  shader.AddDefinitions({{"MaxLightNum", std::to_string(MaxLightNum)}});
  if (static_cast<Vulkan*>(owner)->GetEnableShadowMap()) {
    shader.AddDefinitions({{"EnableShadowMap", std::to_string(1)}});
  }
  if (static_cast<Vulkan*>(owner)->GetEnableDeferred()) {
    shader.AddDefinitions({{"EnableDeferred", std::to_string(1)}});
  }
  if (device.GetMSAASamples() != VK_SAMPLE_COUNT_1_BIT) {
    shader.AddDefinitions(
        {{"EnableMultiSample", std::to_string(device.GetMultiSampleNum())}});
  }

  std::vector<std::string> shaderSearchPaths;
  for (const std::string& searchPath : ShaderSearchPaths) {
    shaderSearchPaths.push_back(rootPath + searchPath);
  }
  shader.SetFileIncluder(shaderSearchPaths);
  shader.SetOptimizationLevel(ShaderOptimizationLevel);
  shader.SetGenerateDebugInfo(
      static_cast<Vulkan*>(owner)->GetEnableShaderDebug());
  pipeline.CreatePipeline(device, render, shader, texCount, rootPath,
                          shaderPaths, zPrePassShaderPath, shadowMapShaderPath);

  if (render.GetEnableDeferred()) {
    CreateDeferredDescriptorPool(device.GetLogical(), render);
    if (auto allLightsChannelPtr =
            static_cast<Vulkan*>(owner)->GetLightChannelByName("All").lock()) {
      allLightsChannelPointer = allLightsChannelPtr.get();
      allLightsChannelBufferPointer =
          &GetBufferManager().CreateLightChannelBuffer(allLightsChannelPointer,
                                                       device, render);
    }
    pipelineBuffer.CreateUniformBuffer(device, render, sizeof(PipelineData));
    CreateDeferredDescriptorSets(device, render);
  }
}

void Draw::LoadDrawResource(const Device& device, Render& render,
                            std::weak_ptr<MeshData> data) {
  Mesh* mesh =
      Create<Mesh>(device, render, data, pipeline.GetColorDescriptorSetLayout(),
                   pipeline.GetZPrePassDescriptorSetLayout(),
                   pipeline.GetShadowMapDescriptorSetLayout());
  meshes.emplace_back(mesh);
}

void Draw::DestroyDrawResource(const VkDevice& device, const Render& render) {
  if (render.GetEnableDeferred()) {
    GetBufferManager().DestroyLightChannelBuffer(allLightsChannelPointer,
                                                 device, render);
    pipelineBuffer.DestroyUniformBuffer(device, render);
    vkDestroyDescriptorPool(device, deferredDescriptorPool, nullptr);
  }

  pipeline.DestroyPipeline(device, render);
  for (Mesh* mesh : meshes) {
    mesh->DestroyMesh(device, render);
    mesh->Destroy();
  }
}

void Draw::Destroy() {
  static_cast<Vulkan*>(owner)->RemoveDrawByShader(shader.GetShaderPath());
  static_cast<Vulkan*>(owner)->RemoveDrawByPipeline(pipeline.GetPipelineId());
  Base::Destroy();
}
void Draw::SetShaderPath(const std::string& shaderPath) {
  shader.SetShaderPath(shaderPath);
}
void Draw::SetPipelineId(const Render& render, const int pipelineId) {
  pipeline.SetPipelineId(pipelineId);
  pipelineBuffer.InitUniformBuffer(render, pipelineId);
}