#include "../include/draw.h"

#include <vector>

#include "../include/config.h"

void Draw::Create(const Device& device,
	const std::string& shaderPath,
	const VkRenderPass& renderPass) {
	pipeline.Create(device, shader, shaderPath, renderPass);
}

void Draw::Load(const Device& device,
	const Render& render,
	const std::vector<std::pair<std::string, std::vector<std::string>>>&
	meshDatas) {
	for (const auto& [dataPath, texPaths]: meshDatas) {
		meshes.emplace_back(device,
			render,
			dataPath,
			texPaths,
			pipeline.GetDescriptorSetLayout());
	}
}

void Draw::Destroy(const VkDevice& device) const {
	pipeline.Destroy(device);
	for (const Mesh& mesh: meshes) {
		mesh.Destroy(device);
	}
}
