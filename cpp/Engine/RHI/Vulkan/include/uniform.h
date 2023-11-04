#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "config.h"
#include "device.h"

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
	int maxFramesInFlight;
	UniformBuffers uniformBuffers;
	UniformMemories uniformBuffersMemory;
	UniformMapped uniformBuffersMapped;

public:
	explicit UniformBuffer() : maxFramesInFlight(Config::MAX_FRAMES_IN_FLIGHT) {
	}

	explicit UniformBuffer(const int maxFrames) : maxFramesInFlight(maxFrames) {
	}

	[[nodiscard]] const int& GetMaxFramesInFlight() const {
		return maxFramesInFlight;
	}

	[[nodiscard]] const UniformBuffers& GetUniformBuffers() const {
		return uniformBuffers;
	}

	void CreateUniformBuffers(const Device& device);

	void UpdateUniformBuffer(const VkExtent2D& swapChainExtent,
		uint32_t currentImage) const;

	void Destroy(const VkDevice& device) const;
};

class Descriptor {
	UniformBuffer uniformBuffer {};
	VkDescriptorPool descriptorPool {};
	DescriptorSets descriptorSets {};

public:
	[[nodiscard]] const UniformBuffer& GetUniformBuffer() const {
		return uniformBuffer;
	}

	[[nodiscard]] const DescriptorSets& GetDescriptorSets() const {
		return descriptorSets;
	}

	[[nodiscard]] const VkDescriptorSet& GetDescriptorSetByIndex(const uint32_t index) const {
		return descriptorSets[index];
	}

	void CreateDescriptorSets(const VkDevice& device,
		const VkDescriptorSetLayout& descriptorSetLayout,
		const Texture& texture);
	void CreateDescriptorPool(const VkDevice& device);

	void CreateUniformBuffers(const Device& device) {
		uniformBuffer.CreateUniformBuffers(device);
	}

	void UpdateUniformBuffers(const VkExtent2D& swapChainExtent,
		const uint32_t currentImage) const {
		uniformBuffer.UpdateUniformBuffer(swapChainExtent, currentImage);
	}

	void Destroy(const VkDevice& device) const;
};
