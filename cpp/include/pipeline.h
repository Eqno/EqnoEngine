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
	 * ������Ⱦ����
	 */
	auto CreateRenderPass(
		const VkFormat& imageFormat,
		const VkDevice& device
	) -> void;
	/**
	 * ����ͼ�ι���
	 */
	auto CreateGraphicsPipeline(
		const Shader&                shader,
		const VkDevice&              device,
		const VkDescriptorSetLayout& descriptorSetLayout
	) -> void;

	/** Destructor And Cleaners **/
	/**
	 * ������Ⱦ����
	 */
	auto DestroyGraphicsPipeline(const VkDevice& device) const -> void;

	/** Getters And Setters **/
	/**
	 * ��ȡ��Ⱦ����
	 */
	[[nodiscard]] auto GetRenderPass() const -> const VkRenderPass& {
		return renderPass;
	}

	/**
	 * ��ȡͼ�ι���
	 */
	[[nodiscard]] auto GetGraphicsPipeline() const -> const VkPipeline& {
		return graphicsPipeline;
	}

	/**
	 * ��ȡ���߲���
	 */
	[[nodiscard]] auto GetPipelineLayout() const -> const VkPipelineLayout& {
		return pipelineLayout;
	}
};
