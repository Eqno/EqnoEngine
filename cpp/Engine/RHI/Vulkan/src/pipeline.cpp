#include "../include/pipeline.h"

#include <stdexcept>
#include <vector>

#include "../include/depth.h"
#include "../include/device.h"
#include "../include/render.h"
#include "../include/shader.h"
#include "../include/uniform.h"
#include "../include/vertex.h"

#define RASTERIZER_CREATE_INFO(_cullMode, _depthBiasEnable)              \
  constexpr VkPipelineRasterizationStateCreateInfo rasterizer {          \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, \
    .depthClampEnable = VK_FALSE, .rasterizerDiscardEnable = VK_FALSE,   \
    .polygonMode = VK_POLYGON_MODE_FILL, .cullMode = _cullMode,          \
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,                        \
    .depthBiasEnable = _depthBiasEnable, .depthBiasConstantFactor = 0,   \
    .depthBiasClamp = 0, .depthBiasSlopeFactor = 0, .lineWidth = 1.0f,   \
  }
#define MULTISAMPLE_CREATE_INFO(_msaaSamples)                              \
  const VkPipelineMultisampleStateCreateInfo multiSampling {               \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,     \
    .rasterizationSamples = _msaaSamples, .sampleShadingEnable = VK_FALSE, \
    .minSampleShading = 1.0f, .pSampleMask = nullptr,                      \
    .alphaToCoverageEnable = VK_FALSE, .alphaToOneEnable = VK_FALSE,       \
  }

#define COLOR_BLEND_ATTACHMENT_STATE()                                        \
  constexpr VkPipelineColorBlendAttachmentState colorBlendAttachment{         \
      .blendEnable = VK_FALSE,                                                \
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,                             \
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,                            \
      .colorBlendOp = VK_BLEND_OP_ADD,                                        \
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,                             \
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,                            \
      .alphaBlendOp = VK_BLEND_OP_ADD,                                        \
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | \
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,  \
  };                                                                          \
  std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(     \
      GBUFFER_SIZE, colorBlendAttachment)
#define COLOR_BLEND_STATE_CREATE_INFO(_attachmentCount, _pAttachments,  \
                                      _logicOp)                         \
  VkPipelineColorBlendStateCreateInfo colorBlending {                   \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,  \
    .logicOpEnable = VK_FALSE, .logicOp = _logicOp,                     \
    .attachmentCount = _attachmentCount, .pAttachments = _pAttachments, \
    .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},                         \
  }
#define DEPTH_STENCIL_STATE_CREATE_INFO(_depthTestEnable, _depthWriteEnable,  \
                                        _depthCompareOp)                      \
  VkPipelineDepthStencilStateCreateInfo depthStencil {                        \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,      \
    .depthTestEnable = _depthTestEnable,                                      \
    .depthWriteEnable = _depthWriteEnable, .depthCompareOp = _depthCompareOp, \
    .depthBoundsTestEnable = VK_FALSE, .stencilTestEnable = VK_FALSE,         \
    .front = {}, .back = {}, .minDepthBounds = 0.0f, .maxDepthBounds = 1.0f,  \
  }

void Pipeline::CreatePipelineLayout(const VkDevice& device,
                                    const VkDescriptorSetLayout& dstLayout,
                                    VkPipelineLayout& pipelineLayout) {
  const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &dstLayout,
  };
  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void Pipeline::CreateColorGraphicsPipeline(
    const Device& device, Render& render, const Shader& shader,
    const std::string& rootPath, const std::vector<std::string>& shaderPaths,
    const VkRenderPass& renderPass) {
  // Forward shading or deferred output pipeline
  auto bindingDescription = Vertex::GetBindingDescription();
  auto attributeDescriptions = Vertex::GetAttributeDescriptions();
  const VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(attributeDescriptions.size()),
      .pVertexAttributeDescriptions = attributeDescriptions.data(),
  };

  constexpr VkPipelineInputAssemblyStateCreateInfo inputAssembly{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };
  constexpr VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
  };
  RASTERIZER_CREATE_INFO(VK_CULL_MODE_BACK_BIT, VK_FALSE);
  MULTISAMPLE_CREATE_INFO(device.GetMSAASamples());
  DEPTH_STENCIL_STATE_CREATE_INFO(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
  if (render.GetEnableZPrePass()) {
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  }
  COLOR_BLEND_ATTACHMENT_STATE();
  COLOR_BLEND_STATE_CREATE_INFO(1, &colorBlendAttachment, VK_LOGIC_OP_COPY);

  if (render.GetEnableDeferred()) {
    colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_NO_OP,
        .attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size()),
        .pAttachments = colorBlendAttachments.data(),
    };
  }

  const std::vector dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                     VK_DYNAMIC_STATE_SCISSOR};
  const VkPipelineDynamicStateCreateInfo dynamicState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };

  CreatePipelineLayout(device.GetLogical(), colorDescriptorSetLayout,
                       colorPipelineLayout);
  if (render.GetEnableDeferred()) {
    CreatePipelineLayout(device.GetLogical(), deferredDescriptorSetLayout,
                         deferredPipelineLayout);
  }

  auto shaderStagesSet(
      shader.AutoCreateStagesSet(device.GetLogical(), rootPath, shaderPaths));

  for (int i = 0; i < shaderStagesSet.size(); ++i) {
    ShaderStages& shaderStages = shaderStagesSet[i];

    auto& stages = shaderStages[PipelineType::Forward];
    if (render.GetEnableDeferred()) {
      stages = shaderStages[PipelineType::DeferredOutputGBuffer];
      if (stages.size() < 2) {
        throw std::runtime_error("missing deferred output shader stages!");
      }
    } else {
      if (stages.size() < 2) {
        throw std::runtime_error("missing forward shader stages!");
      }
    }

    const VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(stages.size()),
        .pStages = stages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multiSampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = colorPipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
    };
    if (vkCreateGraphicsPipelines(device.GetLogical(), VK_NULL_HANDLE, 1,
                                  &pipelineInfo, nullptr,
                                  &colorGraphicsPipeline) == VK_SUCCESS) {
      if (render.GetEnableDeferred()) {
        // Deferred process pipeline
        constexpr VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType =
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .primitiveRestartEnable = VK_FALSE,
        };

        // Remember to initialize its sType
        const VkPipelineVertexInputStateCreateInfo vertexInputInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        };

        VkPipelineColorBlendStateCreateInfo colorBlending{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
        };

        auto& stages = shaderStages[PipelineType::DeferredProcessGBuffer];
        if (stages.size() < 2) {
          throw std::runtime_error("missing deferred process shader stages!");
        }

        const VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = static_cast<uint32_t>(stages.size()),
            .pStages = stages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multiSampling,
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = deferredPipelineLayout,
            .renderPass = renderPass,
            .subpass = 1,
            .basePipelineHandle = VK_NULL_HANDLE,
        };
        if (vkCreateGraphicsPipelines(
                device.GetLogical(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                &deferredGraphicsPipeline) == VK_SUCCESS) {
          shaderFallbackIndex = i;
          shader.DestroyModules(device.GetLogical());
          return;
        }
      } else {
        shaderFallbackIndex = i;
        shader.DestroyModules(device.GetLogical());
        return;
      }
    }
  }
  throw std::runtime_error("failed to create color graphics pipeline!");
}

void Pipeline::CreateZPrePassGraphicsPipeline(
    const Device& device, const Shader& shader, const std::string& rootPath,
    const std::string& depthShaderPath, const VkRenderPass& renderPass) {
  auto bindingDescription = Vertex::GetBindingDescription();
  auto attributeDescriptions = Vertex::GetAttributeDescriptions();
  const VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(attributeDescriptions.size()),
      .pVertexAttributeDescriptions = attributeDescriptions.data(),
  };
  constexpr VkPipelineInputAssemblyStateCreateInfo inputAssembly{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };
  constexpr VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
  };
  RASTERIZER_CREATE_INFO(VK_CULL_MODE_BACK_BIT, VK_FALSE);
  MULTISAMPLE_CREATE_INFO(device.GetMSAASamples());
  DEPTH_STENCIL_STATE_CREATE_INFO(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
  COLOR_BLEND_ATTACHMENT_STATE();
  COLOR_BLEND_STATE_CREATE_INFO(0, nullptr, VK_LOGIC_OP_COPY);
  const std::vector dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                     VK_DYNAMIC_STATE_SCISSOR,
                                     VK_DYNAMIC_STATE_DEPTH_BIAS};
  const VkPipelineDynamicStateCreateInfo dynamicState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };
  CreatePipelineLayout(device.GetLogical(), zPrePassDescriptorSetLayout,
                       zPrePassPipelineLayout);

  ShaderStages shaderStages(
      shader.AutoCreateStages(device.GetLogical(), rootPath, depthShaderPath));
  const VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount =
          static_cast<uint32_t>(shaderStages[PipelineType::Forward].size()),
      .pStages = shaderStages[PipelineType::Forward].data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multiSampling,
      .pDepthStencilState = &depthStencil,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,
      .layout = zPrePassPipelineLayout,
      .renderPass = renderPass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
  };
  if (vkCreateGraphicsPipelines(device.GetLogical(), VK_NULL_HANDLE, 1,
                                &pipelineInfo, nullptr,
                                &zPrePassGraphicsPipeline) == VK_SUCCESS) {
    shader.DestroyModules(device.GetLogical());
    return;
  }
  throw std::runtime_error("failed to create depth graphics pipeline!");
}

void Pipeline::CreateShadowMapGraphicsPipeline(
    const VkDevice& device, const Shader& shader, const std::string& rootPath,
    const std::string& depthShaderPath, const VkRenderPass& renderPass) {
  auto bindingDescription = Vertex::GetBindingDescription();
  auto attributeDescriptions = Vertex::GetAttributeDescriptions();
  const VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(attributeDescriptions.size()),
      .pVertexAttributeDescriptions = attributeDescriptions.data(),
  };
  constexpr VkPipelineInputAssemblyStateCreateInfo inputAssembly{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };
  constexpr VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
  };
  RASTERIZER_CREATE_INFO(VK_CULL_MODE_NONE, VK_TRUE);
  MULTISAMPLE_CREATE_INFO(VK_SAMPLE_COUNT_1_BIT);
  DEPTH_STENCIL_STATE_CREATE_INFO(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
  COLOR_BLEND_ATTACHMENT_STATE();
  COLOR_BLEND_STATE_CREATE_INFO(0, nullptr, VK_LOGIC_OP_COPY);
  const std::vector dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                     VK_DYNAMIC_STATE_SCISSOR,
                                     VK_DYNAMIC_STATE_DEPTH_BIAS};
  const VkPipelineDynamicStateCreateInfo dynamicState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };
  CreatePipelineLayout(device, shadowMapDescriptorSetLayout,
                       shadowMapPipelineLayout);

  ShaderStages shaderStages(
      shader.AutoCreateStages(device, rootPath, depthShaderPath));
  const VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount =
          static_cast<uint32_t>(shaderStages[PipelineType::Forward].size()),
      .pStages = shaderStages[PipelineType::Forward].data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multiSampling,
      .pDepthStencilState = &depthStencil,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,
      .layout = shadowMapPipelineLayout,
      .renderPass = renderPass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
  };
  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                nullptr,
                                &shadowMapGraphicsPipeline) == VK_SUCCESS) {
    shader.DestroyModules(device);
    return;
  }
  throw std::runtime_error("failed to create depth graphics pipeline!");
}

void Pipeline::CreateColorDescriptorSetLayout(const VkDevice& device,
                                              Render& render, int texCount) {
  std::vector<VkDescriptorSetLayoutBinding> bindings;

  if (render.GetEnableDeferred()) {
    // Deferred shading output gBuffer
    for (uint32_t i = 0; i < UniformBufferNum - 1; i++) {
      bindings.push_back({
          .binding = static_cast<uint32_t>(bindings.size()),
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount = 1,
          .stageFlags =
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
          .pImmutableSamplers = nullptr,
      });
    }
    for (uint32_t i = 0; i < texCount; i++) {
      bindings.push_back({
          .binding = static_cast<uint32_t>(bindings.size()),
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
          .pImmutableSamplers = nullptr,
      });
    }
    VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                    &colorDescriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create descriptor set layout!");
    }

    // Deferred shading process gBuffer
    bindings.clear();
    bindings.push_back({
        .binding = static_cast<uint32_t>(bindings.size()),
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr,
    });
    for (uint32_t i = 0; i < GBUFFER_SIZE; i++) {
      bindings.push_back({
          .binding = static_cast<uint32_t>(bindings.size()),
          .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
          .pImmutableSamplers = nullptr,
      });
    }
    if (render.GetEnableShadowMap()) {
      bindings.push_back({
          .binding = static_cast<uint32_t>(bindings.size()),
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount =
              static_cast<uint32_t>(render.GetShadowMapDepthNum()),
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
          .pImmutableSamplers = nullptr,
      });
    }
    layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                    &deferredDescriptorSetLayout) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to create descriptor set layout!");
    }
  } else {
    // Forward shading
    for (uint32_t i = 0; i < UniformBufferNum; i++) {
      bindings.push_back({
          .binding = static_cast<uint32_t>(bindings.size()),
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount = 1,
          .stageFlags =
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
          .pImmutableSamplers = nullptr,
      });
    }
    if (render.GetEnableShadowMap()) {
      bindings.push_back({
          .binding = static_cast<uint32_t>(bindings.size()),
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount =
              static_cast<uint32_t>(render.GetShadowMapDepthNum()),
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
          .pImmutableSamplers = nullptr,
      });
    }
    for (uint32_t i = 0; i < texCount; i++) {
      bindings.push_back({
          .binding = static_cast<uint32_t>(bindings.size()),
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
          .pImmutableSamplers = nullptr,
      });
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                    &colorDescriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create descriptor set layout!");
    }
  }
}

void Pipeline::CreateZPrePassDescriptorSetLayout(const VkDevice& device) {
  VkDescriptorSetLayoutBinding binding{
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .pImmutableSamplers = nullptr,
  };
  VkDescriptorSetLayoutCreateInfo layoutInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings = &binding,
  };
  if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                  &zPrePassDescriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout!");
  }
}

void Pipeline::CreateShadowMapDescriptorSetLayout(const VkDevice& device) {
  VkDescriptorSetLayoutBinding binding{
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .pImmutableSamplers = nullptr,
  };
  VkDescriptorSetLayoutCreateInfo layoutInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings = &binding,
  };
  if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                  &shadowMapDescriptorSetLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout!");
  }
}

void Pipeline::CreatePipeline(const Device& device, Render& render,
                              const Shader& shader, int texCount,
                              const std::string& rootPath,
                              const std::vector<std::string>& shaderPaths,
                              const std::string& zPrePassShaderPath,
                              const std::string& shadowMapShaderPath) {
  CreateColorDescriptorSetLayout(device.GetLogical(), render, texCount);
  CreateColorGraphicsPipeline(device, render, shader, rootPath, shaderPaths,
                              render.GetColorRenderPass());

  if (render.GetEnableZPrePass()) {
    CreateZPrePassDescriptorSetLayout(device.GetLogical());
    CreateZPrePassGraphicsPipeline(device, shader, rootPath, zPrePassShaderPath,
                                   render.GetZPrePassRenderPass());
  }
  if (render.GetEnableShadowMap()) {
    CreateShadowMapDescriptorSetLayout(device.GetLogical());
    CreateShadowMapGraphicsPipeline(device.GetLogical(), shader, rootPath,
                                    shadowMapShaderPath,
                                    render.GetShadowMapRenderPass());
  }
}

void Pipeline::DestroyPipeline(const VkDevice& device,
                               const Render& render) const {
  vkDestroyPipeline(device, colorGraphicsPipeline, nullptr);
  vkDestroyDescriptorSetLayout(device, colorDescriptorSetLayout, nullptr);
  vkDestroyPipelineLayout(device, colorPipelineLayout, nullptr);

  if (render.GetEnableZPrePass()) {
    vkDestroyPipeline(device, zPrePassGraphicsPipeline, nullptr);
    vkDestroyDescriptorSetLayout(device, zPrePassDescriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(device, zPrePassPipelineLayout, nullptr);
  }
  if (render.GetEnableShadowMap()) {
    vkDestroyPipeline(device, shadowMapGraphicsPipeline, nullptr);
    vkDestroyDescriptorSetLayout(device, shadowMapDescriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(device, shadowMapPipelineLayout, nullptr);
  }
  if (render.GetEnableDeferred()) {
    vkDestroyPipeline(device, deferredGraphicsPipeline, nullptr);
    vkDestroyDescriptorSetLayout(device, deferredDescriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(device, deferredPipelineLayout, nullptr);
  }
}

#undef DEPTH_STENCIL_STATE_CREATE_INFO
#undef COLOR_BLEND_STATE_CREATE_INFO
#undef COLOR_BLEND_ATTACHMENT_STATE
#undef MULTISAMPLE_CREATE_INFO
#undef RASTERIZER_CREATE_INFO