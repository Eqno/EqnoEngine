#include "../include/pipeline.h"

#include <stdexcept>
#include <vector>

#include "../include/depth.h"
#include "../include/device.h"
#include "../include/shader.h"
#include "../include/uniform.h"
#include "../include/vertex.h"

void Pipeline::CreateGraphicsPipeline(const VkDevice& device,
                                      const Shader& shader,
                                      const std::string& shaderPath,
                                      const VkRenderPass& renderPass) {
  auto shaderStages(shader.AutoCreateStages(device, shaderPath));
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
  constexpr VkPipelineRasterizationStateCreateInfo rasterizer{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f,
  };
  constexpr VkPipelineMultisampleStateCreateInfo multiSampling{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
  };
  VkPipelineDepthStencilStateCreateInfo depthStencil{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE,
  };
  constexpr VkPipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };
  VkPipelineColorBlendStateCreateInfo colorBlending{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
      .blendConstants =
          {
              0.0f,
              0.0f,
              0.0f,
              0.0f,
          },
  };
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
      .pSetLayouts = &descriptorSetLayout,
  };
  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
  const VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = 2,
      .pStages = shaderStages.data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multiSampling,
      .pDepthStencilState = &depthStencil,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,
      .layout = pipelineLayout,
      .renderPass = renderPass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
  };
  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                nullptr, &graphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }
  shader.DestroyModules(device);
}

void Pipeline::CreateDescriptorSetLayout(const VkDevice& device, int texCount) {
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  for (unsigned int i = 0; i < UniformBufferNum - 1; i++) {
    bindings.push_back({
        .binding = i,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr,
    });
  }
  bindings.push_back({
      .binding = UniformBufferNum - 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
  });
  for (unsigned int i = 0; i < texCount; i++) {
    bindings.emplace_back(VkDescriptorSetLayoutBinding{
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
                                  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout!");
  }
}

void Pipeline::CreatePipeline(const Device& device, const Shader& shader,
                              const std::string& shaderPath,
                              const VkRenderPass& renderPass, int texCount) {
  CreateDescriptorSetLayout(device.GetLogical(), texCount);
  CreateGraphicsPipeline(device.GetLogical(), shader, shaderPath, renderPass);
}

void Pipeline::DestroyPipeline(const VkDevice& device) const {
  vkDestroyPipeline(device, graphicsPipeline, nullptr);
  vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
  vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}
