#include "../include/mesh.h"

void Mesh::Create(const Device& device,
	const Render& render,
	const std::string& dataPath,
	const std::vector<std::string>& texPaths,
	const VkDescriptorSetLayout& descriptorSetLayout) {
	for (const std::string& texPath: texPaths) {
		textures.emplace_back(device, render, texPath.c_str());
	}
	data.Create(dataPath.c_str());
	buffer.Create(device, data.GetVertices(), data.GetIndices(), render);
	descriptor.Create(device, render, descriptorSetLayout, textures);
}

void Mesh::Destroy(const VkDevice& device, const Render& render) const {
	descriptor.Destroy(device, render);
	buffer.Destroy(device);
	for (const Texture& texture: textures) {
		texture.Destroy(device);
	}
}
