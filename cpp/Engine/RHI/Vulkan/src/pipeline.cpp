#include "../include/pipeline.h"

#include <stdexcept>

#include "../include/depth.h"
#include "../include/device.h"
#include "../include/vertex.h"

void Pipeline::CreateRenderPass(const VkFormat& imageFormat,
	const Device& device) {
	const VkAttachmentDescription colorAttachment {
		.format = imageFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};
	const VkAttachmentDescription depthAttachment {
		.format = Depth::FindDepthFormat(device.GetPhysical()),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	constexpr VkAttachmentReference colorAttachmentRef {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	constexpr VkAttachmentReference depthAttachmentRef {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	VkSubpassDescription subPass {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
		.pDepthStencilAttachment = &depthAttachmentRef,
	};
	constexpr VkSubpassDependency dependency {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	};
	std::array attachments = {colorAttachment, depthAttachment};
	const VkRenderPassCreateInfo renderPassInfo {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = static_cast<uint32_t>(attachments.size()),
		.pAttachments = attachments.data(),
		.subpassCount = 1,
		.pSubpasses = &subPass,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};
	if (vkCreateRenderPass(device.GetLogical(),
		    &renderPassInfo,
		    nullptr,
		    &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void Pipeline::CreateGraphicsPipeline(const Shader& shader,
	const VkDevice& device) {
	auto shaderStages(shader.AutoCreateStages(device));
	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescriptions = Vertex::GetAttributeDescriptions();
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(
			attributeDescriptions.size()),
		.pVertexAttributeDescriptions = attributeDescriptions.data(),
	};
	constexpr VkPipelineInputAssemblyStateCreateInfo inputAssembly {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};
	constexpr VkPipelineViewportStateCreateInfo viewportState {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};
	constexpr VkPipelineRasterizationStateCreateInfo rasterizer {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1.0f,
	};
	constexpr VkPipelineMultisampleStateCreateInfo multiSampling {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
	};
	VkPipelineDepthStencilStateCreateInfo depthStencil {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
	};
	constexpr VkPipelineColorBlendAttachmentState colorBlendAttachment {
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};
	constexpr VkPipelineColorBlendStateCreateInfo colorBlending {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = {0.0f, 0.0f, 0.0f, 0.0f,},
	};
	const std::vector dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	const VkPipelineDynamicStateCreateInfo dynamicState {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data(),
	};
	const VkPipelineLayoutCreateInfo pipelineLayoutInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptorSetLayout,
	};
	if (vkCreatePipelineLayout(device,
		    &pipelineLayoutInfo,
		    nullptr,
		    &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
	const VkGraphicsPipelineCreateInfo pipelineInfo {
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
	if (vkCreateGraphicsPipelines(device,
		    VK_NULL_HANDLE,
		    1,
		    &pipelineInfo,
		    nullptr,
		    &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	shader.DestroyModules(device);
}

void Pipeline::DestroyGraphicsPipeline(const VkDevice& device) const {
	vkDestroyPipeline(device, graphicsPipeline, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
}

void Pipeline::CreateDescriptorSetLayout(const VkDevice& device) {
	constexpr VkDescriptorSetLayoutBinding uboLayoutBinding{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr,
	};

	constexpr VkDescriptorSetLayoutBinding samplerLayoutBinding{
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr,
	};

	std::array bindings = { uboLayoutBinding, samplerLayoutBinding };

	constexpr VkDescriptorSetLayoutCreateInfo layoutInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data(),
	};

	if (vkCreateDescriptorSetLayout(device,
		&layoutInfo,
		nullptr,
		&descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}