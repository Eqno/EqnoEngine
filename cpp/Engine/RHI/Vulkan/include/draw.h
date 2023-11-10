#pragma once

#include "mesh.h"
#include "pipeline.h"
#include "shader.h"

class Render;

class Draw {
	Shader shader;
	Pipeline pipeline;
	std::vector<Mesh> meshes;

public:
	[[nodiscard]] const VkPipeline& GetGraphicsPipeline() const {
		return pipeline.GetGraphicsPipeline();
	}

	[[nodiscard]] const VkPipelineLayout& GetPipelineLayout() const {
		return pipeline.GetPipelineLayout();
	}

	[[nodiscard]] const VkDescriptorSetLayout& GetDescriptorSetLayout() const {
		return pipeline.GetDescriptorSetLayout();
	}

	const std::vector<Mesh>& GetMeshes() const {
		return meshes;
	}

	Draw() = default;

	Draw(const Device& device,
		const std::string& shaderPath,
		const VkRenderPass& renderPass) {
		Create(device, shaderPath, renderPass);
	}

	void Create(const Device& device,
		const std::string& shaderPath,
		const VkRenderPass& renderPass);

	void Load(const Device& device, const Render& render, const MeshData* data);

	void Destroy(const VkDevice& device) const;
};
