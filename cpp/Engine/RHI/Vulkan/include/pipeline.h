#pragma once

#include <vulkan/vulkan_core.h>

#include "shader.h"

class Device;

class Pipeline {
	VkRenderPass renderPass {};

	VkDescriptorSetLayout descriptorSetLayout {};
	VkPipelineLayout pipelineLayout {};
	VkPipeline graphicsPipeline {};

public:
	void CreateRenderPass(const VkFormat& imageFormat, const Device& device);
	void CreateGraphicsPipeline(const Shader& shader, const VkDevice& device);
	void DestroyGraphicsPipeline(const VkDevice& device) const;
	void CreateDescriptorSetLayout(const VkDevice& device);

	[[nodiscard]] const VkRenderPass& GetRenderPass() const {
		return renderPass;
	}

	[[nodiscard]] const VkPipeline& GetGraphicsPipeline() const {
		return graphicsPipeline;
	}

	[[nodiscard]] const VkPipelineLayout& GetPipelineLayout() const {
		return pipelineLayout;
	}

	[[nodiscard]] const VkDescriptorSetLayout& GetDescriptorSetLayout() const {
		return descriptorSetLayout;
	}
};
