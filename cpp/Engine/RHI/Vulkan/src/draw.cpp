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
	const MeshData* data) {
	meshes.emplace_back(device, render, data,
		pipeline.GetDescriptorSetLayout());
}

void Draw::Destroy(const VkDevice& device) const {
	pipeline.Destroy(device);
	for (const Mesh& mesh: meshes) {
		mesh.Destroy(device);
	}
}
