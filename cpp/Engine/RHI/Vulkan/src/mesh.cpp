#include "../include/mesh.h"

void Mesh::Create(const Device& device,
	const Render& render,
	const MeshData* inData,
	const VkDescriptorSetLayout& descriptorSetLayout) {
	ParseTextures(device, render, inData);
	ParseVertexAndIndex(inData);
	ParseBufferAndDescriptor(device, render, descriptorSetLayout);
}

void Mesh::Destroy(const VkDevice& device, const Render& render) const {
	descriptor.Destroy(device, render);
	buffer.Destroy(device);
	for (const Texture& texture: textures) {
		texture.Destroy(device);
	}
}

void Mesh::ParseTextures(const Device& device,
	const Render& render,
	const MeshData* inData) {
	for (const auto& [width, height, channels, _data]: inData->textures) {
		textures.emplace_back("NOSRPGB",
			device,
			render,
			width,
			height,
			channels,
			_data);
	}
}

void Mesh::ParseVertexAndIndex(const MeshData* inData) {
	std::vector<Vertex> vertices;
	vertices.reserve(inData->vertices.size());
	for (const auto& vert: inData->vertices) {
		vertices.emplace_back(vert);
	}
	data.Create(inData->indices, vertices);
}

void Mesh::ParseBufferAndDescriptor(const Device& device,
	const Render& render,
	const VkDescriptorSetLayout& descriptorSetLayout) {
	buffer.Create(device, data.GetVertices(), data.GetIndices(), render);
	descriptor.Create(device, render, descriptorSetLayout, textures);
}
