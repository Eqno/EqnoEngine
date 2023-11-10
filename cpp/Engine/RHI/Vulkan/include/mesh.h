#pragma once

#include "buffer.h"
#include "data.h"
#include "uniform.h"
#include "texture.h"
#include "Engine/Utility/include/TypeUtils.h"

class Mesh {
	Data data;
	Buffer buffer;
	Descriptor descriptor;
	std::vector<Texture> textures;

	void ParseTextures(const Device& device,
		const Render& render,
		const MeshData* inData);

	void ParseVertexAndIndex(const MeshData* inData);

	void ParseBufferAndDescriptor(const Device& device,
		const Render& render,
		const VkDescriptorSetLayout& descriptorSetLayout);

public:
	[[nodiscard]] const VkBuffer& GetIndexBuffer() const {
		return buffer.GetIndexBuffer();
	}

	[[nodiscard]] const VkBuffer& GetVertexBuffer() const {
		return buffer.GetVertexBuffer();
	}

	[[nodiscard]] const std::vector<uint32_t>& GetIndices() const {
		return data.GetIndices();
	}

	[[nodiscard]] const std::vector<Vertex>& GetVertices() const {
		return data.GetVertices();
	}

	[[nodiscard]] const DescriptorSets& GetDescriptorSets() const {
		return descriptor.GetDescriptorSets();
	}

	[[nodiscard]] const VkDescriptorSet& GetDescriptorSetByIndex(
		const uint32_t index) const {
		return descriptor.GetDescriptorSetByIndex(index);
	}

	Mesh(const Device& device,
		const Render& render,
		const MeshData* data,
		const VkDescriptorSetLayout& descriptorSetLayout) {
		Create(device, render, data, descriptorSetLayout);
	}

	void Create(const Device& device,
		const Render& render,
		const MeshData* inData,
		const VkDescriptorSetLayout& descriptorSetLayout);

	void Destroy(const VkDevice& device) const;
};
