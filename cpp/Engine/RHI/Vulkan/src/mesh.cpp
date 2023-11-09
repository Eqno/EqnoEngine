#include "../include/mesh.h"

void Mesh::Create(const Device& device,
	const Render& render,
	const std::string& dataPath,
	const std::vector<std::string>& texPaths,
	const VkDescriptorSetLayout& descriptorSetLayout) {
	for (const std::string& texPath: texPaths) {
		textures.emplace_back("NOSRPGB", device, render, texPath.c_str());
	}
	data.Create(dataPath);
	buffer.Create(device, data.GetVertices(), data.GetIndices(), render);
	descriptor.Create(device, render, descriptorSetLayout, textures);
}

void Mesh::Create(const Device& device,
	const Render& render,
	const MeshData* inData,
	const VkDescriptorSetLayout& descriptorSetLayout) {
	for (const TextureData& texData: inData->textures) {
		textures.emplace_back("NOSRPGB", device, render, texData.width,
			texData.height, texData.channels, texData.data);
	}
	data.Create(inData->indices, inData->vertices);
}


void Mesh::Destroy(const VkDevice& device) const {
	descriptor.Destroy(device);
	buffer.Destroy(device);
	for (const Texture& texture: textures) {
		texture.Destroy(device);
	}
}
