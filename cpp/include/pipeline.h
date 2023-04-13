#pragma once

class Shader;

class Pipeline {
	VkRenderPass     renderPass {};
	VkPipelineLayout pipelineLayout {};
	VkPipeline       graphicsPipeline {};

public:
	auto CreateRenderPass(
		const VkFormat& imageFormat,
		const VkDevice& device
	) -> void;

	auto CreateGraphicsPipeline(
		const Shader&                shader,
		const VkDevice&              device,
		const VkDescriptorSetLayout& descriptorSetLayout
	) -> void;

	auto DestroyGraphicsPipeline(const VkDevice& device) const -> void;

	[[nodiscard]] auto GetRenderPass() const -> const VkRenderPass& {
		return renderPass;
	}

	[[nodiscard]] auto GetGraphicsPipeline() const -> const VkPipeline& {
		return graphicsPipeline;
	}

	[[nodiscard]] auto GetPipelineLayout() const -> const VkPipelineLayout& {
		return pipelineLayout;
	}
};
