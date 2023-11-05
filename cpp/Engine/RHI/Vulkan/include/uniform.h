#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <vulkan/vulkan_core.h>

class Device;
class Render;
class Texture;

using UniformMapped = std::vector<void*>;
using UniformBuffers = std::vector<VkBuffer>;
using UniformMemories = std::vector<VkDeviceMemory>;
using DescriptorSets = std::vector<VkDescriptorSet>;

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class UniformBuffer {
	UniformBuffers uniformBuffers;
	UniformMemories uniformBuffersMemory;
	UniformMapped uniformBuffersMapped;

public:
	explicit UniformBuffer() = default;

	[[nodiscard]] const UniformBuffers& GetUniformBuffers() const {
		return uniformBuffers;
	}

	void CreateUniformBuffers(const Device& device, const Render& render);

	void UpdateUniformBuffer(const VkExtent2D& swapChainExtent,
		uint32_t currentImage) const;

	void Destroy(const VkDevice& device, const Render& render) const;
};

class Descriptor {
	UniformBuffer uniformBuffer {};
	VkDescriptorPool descriptorPool {};
	DescriptorSets descriptorSets {};

	void CreateDescriptorSets(const VkDevice& device,
		const Render& render,
		const VkDescriptorSetLayout& descriptorSetLayout,
		const std::vector<Texture>& textures);

	void CreateDescriptorPool(const VkDevice& device,
		const Render& render,
		const size_t textureNum);

	void CreateUniformBuffers(const Device& device, const Render& render) {
		uniformBuffer.CreateUniformBuffers(device, render);
	}

public:
	[[nodiscard]] const UniformBuffer& GetUniformBuffer() const {
		return uniformBuffer;
	}

	[[nodiscard]] const DescriptorSets& GetDescriptorSets() const {
		return descriptorSets;
	}

	[[nodiscard]] const VkDescriptorSet& GetDescriptorSetByIndex(
		const uint32_t index) const {
		return descriptorSets[index];
	}

	void UpdateUniformBuffers(const VkExtent2D& swapChainExtent,
		const uint32_t currentImage) const {
		uniformBuffer.UpdateUniformBuffer(swapChainExtent, currentImage);
	}

	void Create(const Device& device,
		const Render& render,
		const VkDescriptorSetLayout& descriptorSetLayout,
		const std::vector<Texture>& textures);
	void Destroy(const VkDevice& device, const Render& render) const;
};