#pragma once

#include <vulkan/vulkan_core.h>

#include "shader.h"

class Pipeline {
	VkRenderPass     renderPass {};
	VkPipelineLayout pipelineLayout {};
	VkPipeline       graphicsPipeline {};
public:
	/** Pickers And Creators **/
	/**
	 * 创建渲染流程
	 */
	void CreateRenderPass(
		const VkFormat& imageFormat,
		const VkDevice& device
	);
	/**
	 * 创建图形管线
	 */
	auto CreateGraphicsPipeline(
		const Shader&                shader,
		const VkDevice&              device,
		const VkDescriptorSetLayout& descriptorSetLayout
	) -> void;

	/** Destructor And Cleaners **/
	/**
	 * 销毁渲染流程
	 */
	auto DestroyGraphicsPipeline(const VkDevice& device) const -> void;

	/** Getters And Setters **/
	/**
	 * 获取渲染流程
	 */
	[[nodiscard]] auto GetRenderPass() const -> const VkRenderPass& {
		return renderPass;
	}

	/**
	 * 获取图形管线
	 */
	[[nodiscard]] auto GetGraphicsPipeline() const -> const VkPipeline& {
		return graphicsPipeline;
	}

	/**
	 * 获取管线布局
	 */
	[[nodiscard]] auto GetPipelineLayout() const -> const VkPipelineLayout& {
		return pipelineLayout;
	}
};
