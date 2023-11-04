#include "../include/mesh.h"

void Mesh::Create(const Device& device,
	const Render& render,
	const Pipeline& pipeline) {
	textures.emplace_back(device, render, Config::TEXTURE_PATH.c_str());
	data.Create(Config::MODEL_PATH.c_str());

	buffer.Create(device, data.GetVertices(), data.GetIndices(), render);
	descriptor.Create(device,
		render,
		pipeline.GetDescriptorSetLayout(),
		textures);
}

void Mesh::Destroy(const VkDevice& device, const Render& render) const {
	descriptor.Destroy(device, render);
	buffer.Destroy(device);
	for (const Texture& texture: textures) {
		texture.Destroy(device);
	}
}
