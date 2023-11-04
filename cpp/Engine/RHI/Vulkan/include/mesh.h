#pragma once

#include "buffer.h"
#include "data.h"
#include "texture.h"
#include "uniform.h"

class Mesh {
	Data data;
	Buffer buffer;
	Descriptor descriptor;
	std::vector<Texture> textures;

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

	[[nodiscard]] const UniformBuffer& GetUniformBuffer() const {
		return descriptor.GetUniformBuffer();
	}

	void UpdateUniformBuffers(const VkExtent2D& swapChainExtent,
		const uint32_t currentImage) const {
		descriptor.UpdateUniformBuffers(swapChainExtent, currentImage);
	}

	void Create(const Device& device,
		const Render& render,
		const Pipeline& pipeline);

	void Destroy(const VkDevice& device) const;
};
