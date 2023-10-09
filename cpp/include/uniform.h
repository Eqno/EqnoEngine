#pragma once

#define GLM_FORCE_RADIANS // NOLINT(clang-diagnostic-unused-macros)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <vulkan/vulkan_core.h>

#include "config.h"
#include "device.h"

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
	const int maxFramesInFlight;
	UniformBuffers uniformBuffers;
	UniformMemories uniformBuffersMemory;
	UniformMapped uniformBuffersMapped;

public:
	explicit UniformBuffer() : maxFramesInFlight(Config::MAX_FRAMES_IN_FLIGHT) {}

	explicit
	UniformBuffer(const int maxFrames) : maxFramesInFlight(maxFrames) {}

	[[nodiscard]] const int& GetMaxFramesInFlight() const {
		return maxFramesInFlight;
	}

	[[nodiscard]] const UniformBuffers& GetUniformBuffers() const {
		return uniformBuffers;
	}

	void CreateUniformBuffers(const Device& device);

	void UpdateUniformBuffer(
		const VkExtent2D& swapChainExtent,
		uint32_t currentImage
	) const;

	void Destroy(const VkDevice& device) const;
};

class Descriptor {
	UniformBuffer uniformBuffer {};
	VkDescriptorPool descriptorPool {};
	VkDescriptorSetLayout descriptorSetLayout {};
	DescriptorSets descriptorSets {};

public:
	[[nodiscard]] const VkDescriptorSetLayout& GetSetLayout() const {
		return descriptorSetLayout;
	}

	[[nodiscard]] const DescriptorSets& GetDescriptorSets() const {
		return descriptorSets;
	}

	void CreateDescriptorSetLayout(const VkDevice& device);
	void CreateDescriptorSets(const VkDevice& device);
	void CreateDescriptorPool(const VkDevice& device);

	void CreateUniformBuffers(const Device& device) {
		uniformBuffer.CreateUniformBuffers(device);
	}

	void UpdateUniformBuffers(
		const VkExtent2D& swapChainExtent,
		const uint32_t currentImage
	) const {
		uniformBuffer.UpdateUniformBuffer(swapChainExtent, currentImage);
	}

	void Destroy(const VkDevice& device) const;
};
