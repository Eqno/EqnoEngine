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
#define MULTISAMPLE_CREATE_INFO()                                      \
  constexpr VkPipelineMultisampleStateCreateInfo multiSampling {       \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, \
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,                     \
    .sampleShadingEnable = VK_FALSE, .minSampleShading = 1.0f,         \
    .pSampleMask = nullptr, .alphaToCoverageEnable = VK_FALSE,         \
    .alphaToOneEnable = VK_FALSE,                                      \
  }

#define COLOR_BLEND_ATTACHMENT_STATE()                                      \
  constexpr VkPipelineColorBlendAttachmentState colorBlendAttachment {      \
    .blendEnable = VK_FALSE, .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,    \
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,                            \
    .colorBlendOp = VK_BLEND_OP_ADD,                                        \
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,                             \
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,                            \
    .alphaBlendOp = VK_BLEND_OP_ADD,                                        \
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | \
                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,  \
  }
#define COLOR_BLEND_STATE_CREATE_INFO(_attachmentCount, _pAttachments)  \
  VkPipelineColorBlendStateCreateInfo colorBlending {                   \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,  \
    .logicOpEnable = VK_FALSE, .logicOp = VK_LOGIC_OP_COPY,             \
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

void Pipeline::CreateColorGraphicsPipeline(
    const VkDevice& device, const Shader& shader, const std::string& rootPath,
    const std::vector<std::string>& shaderPaths,
    const VkRenderPass& renderPass) {
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
  MULTISAMPLE_CREATE_INFO();
  DEPTH_STENCIL_STATE_CREATE_INFO(VK_TRUE, VK_FALSE,
                                  VK_COMPARE_OP_LESS_OR_EQUAL);
  COLOR_BLEND_ATTACHMENT_STATE();
  COLOR_BLEND_STATE_CREATE_INFO(1, &colorBlendAttachment);

  const std::vector dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                     VK_DYNAMIC_STATE_SCISSOR};
  const VkPipelineDynamicStateCreateInfo dynamicState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };
  const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &colorDescriptorSetLayout,
  };
  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                             &colorPipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  auto shaderStagesSet(
      shader.AutoCreateStagesSet(device, rootPath, shaderPaths));
  for (int i = 0; i < shaderStagesSet.size(); ++i) {
    ShaderStages& shaderStages = shaderStagesSet[i];
    const VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
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
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                  nullptr,
                                  &colorGraphicsPipeline) == VK_SUCCESS) {
      shaderFallbackIndex = i;
      shader.DestroyModules(device);
      return;
    }
  }
  throw std::runtime_error("failed to create color graphics pipeline!");
}

void Pipeline::CreateZPrePassGraphicsPipeline(
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
  RASTERIZER_CREATE_INFO(VK_CULL_MODE_BACK_BIT, VK_FALSE);
  MULTISAMPLE_CREATE_INFO();
  DEPTH_STENCIL_STATE_CREATE_INFO(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
  COLOR_BLEND_ATTACHMENT_STATE();
  COLOR_BLEND_STATE_CREATE_INFO(0, nullptr);
  const std::vector dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                     VK_DYNAMIC_STATE_SCISSOR,
                                     VK_DYNAMIC_STATE_DEPTH_BIAS};
  const VkPipelineDynamicStateCreateInfo dynamicState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };
  const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &zPrePassDescriptorSetLayout,
  };
  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                             &zPrePassPipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  ShaderStages shaderStages(
      shader.AutoCreateStages(device, rootPath, depthShaderPath));
  const VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = static_cast<uint32_t>(shaderStages.size()),
      .pStages = shaderStages.data(),
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
  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                nullptr,
                                &zPrePassGraphicsPipeline) == VK_SUCCESS) {
    shader.DestroyModules(device);
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
  MULTISAMPLE_CREATE_INFO();
  DEPTH_STENCIL_STATE_CREATE_INFO(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
  COLOR_BLEND_ATTACHMENT_STATE();
  COLOR_BLEND_STATE_CREATE_INFO(0, nullptr);
  const std::vector dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                     VK_DYNAMIC_STATE_SCISSOR,
                                     VK_DYNAMIC_STATE_DEPTH_BIAS};
  const VkPipelineDynamicStateCreateInfo dynamicState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };
  const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &shadowMapDescriptorSetLayout,
  };
  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                             &shadowMapPipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  ShaderStages shaderStages(
      shader.AutoCreateStages(device, rootPath, depthShaderPath));
  const VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = static_cast<uint32_t>(shaderStages.size()),
      .pStages = shaderStages.data(),
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
  for (unsigned int i = 0; i < UniformBufferNum; i++) {
    bindings.push_back({
        .binding = i,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr,
    });
  }
  bindings.push_back({
      .binding = UniformBufferNum,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = static_cast<uint32_t>(render.GetShadowMapDepthNum()),
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      .pImmutableSamplers = nullptr,
  });
  for (unsigned int i = 1; i < 1 + texCount; i++) {
    bindings.push_back({
        .binding = i + UniformBufferNum,
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
  CreateZPrePassDescriptorSetLayout(device.GetLogical());
  CreateShadowMapDescriptorSetLayout(device.GetLogical());

  CreateColorGraphicsPipeline(device.GetLogical(), shader, rootPath,
                              shaderPaths, render.GetColorRenderPass());
  CreateZPrePassGraphicsPipeline(device.GetLogical(), shader, rootPath,
                                 zPrePassShaderPath,
                                 render.GetZPrePassRenderPass());
  CreateShadowMapGraphicsPipeline(device.GetLogical(), shader, rootPath,
                                  shadowMapShaderPath,
                                  render.GetShadowMapRenderPass());
}

void Pipeline::DestroyPipeline(const VkDevice& device) const {
  vkDestroyPipeline(device, colorGraphicsPipeline, nullptr);
  vkDestroyPipeline(device, zPrePassGraphicsPipeline, nullptr);
  vkDestroyPipeline(device, shadowMapGraphicsPipeline, nullptr);

  vkDestroyDescriptorSetLayout(device, colorDescriptorSetLayout, nullptr);
  vkDestroyDescriptorSetLayout(device, zPrePassDescriptorSetLayout, nullptr);
  vkDestroyDescriptorSetLayout(device, shadowMapDescriptorSetLayout, nullptr);

  vkDestroyPipelineLayout(device, colorPipelineLayout, nullptr);
  vkDestroyPipelineLayout(device, zPrePassPipelineLayout, nullptr);
  vkDestroyPipelineLayout(device, shadowMapPipelineLayout, nullptr);
}

#undef DEPTH_STENCIL_STATE_CREATE_INFO
#undef COLOR_BLEND_STATE_CREATE_INFO
#undef COLOR_BLEND_ATTACHMENT_STATE
#undef MULTISAMPLE_CREATE_INFO
#undef RASTERIZER_CREATE_INFO