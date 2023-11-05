#pragma once

#include <string>
#include <vulkan/vulkan_core.h>

class Shader;
class Device;

class Pipeline {
	VkDescriptorSetLayout descriptorSetLayout {};
	VkPipelineLayout pipelineLayout {};
	VkPipeline graphicsPipeline {};

	void CreateGraphicsPipeline(const VkDevice& device,
		const Shader& shader,
		const std::string& shaderPath,
		const VkRenderPass& renderPass);
	void CreateDescriptorSetLayout(const VkDevice& device);

public:
	[[nodiscard]] const VkPipeline& GetGraphicsPipeline() const {
		return graphicsPipeline;
	}

	[[nodiscard]] const VkPipelineLayout& GetPipelineLayout() const {
		return pipelineLayout;
	}

	[[nodiscard]] const VkDescriptorSetLayout& GetDescriptorSetLayout() const {
		return descriptorSetLayout;
	}

	void Create(const Device& device,
		const Shader& shader,
		const std::string& shaderPath,
		const VkRenderPass& renderPass);
	void Destroy(const VkDevice& device) const;
};
