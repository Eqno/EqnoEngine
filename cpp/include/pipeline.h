#pragma once

#include <vulkan/vulkan_core.h>

#include "shader.h"

class Device;

class Pipeline {
	VkRenderPass renderPass {};
	VkPipelineLayout pipelineLayout {};
	VkPipeline graphicsPipeline {};

public:
	/** Pickers And Creators **/
	/**
	 * 创建渲染流程
	 */
	void CreateRenderPass(const VkFormat& imageFormat, const Device& device);
	/**
	 * 创建图形管线
	 */
	void CreateGraphicsPipeline(const Shader& shader,
		const VkDevice& device,
		const VkDescriptorSetLayout& descriptorSetLayout);

	/** Destructor And Cleaners **/
	/**
	 * 销毁渲染流程
	 */
	void DestroyGraphicsPipeline(const VkDevice& device) const;

	/** Getters And Setters **/
	/**
	 * 获取渲染流程
	 */
	[[nodiscard]] const VkRenderPass& GetRenderPass() const {
		return renderPass;
	}

	/**
	 * 获取图形管线
	 */
	[[nodiscard]] const VkPipeline& GetGraphicsPipeline() const {
		return graphicsPipeline;
	}

	/**
	 * 获取管线布局
	 */
	[[nodiscard]] const VkPipelineLayout& GetPipelineLayout() const {
		return pipelineLayout;
	}
};
