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
	const int       maxFramesInFlight;
	UniformBuffers  uniformBuffers;
	UniformMemories uniformBuffersMemory;
	UniformMapped   uniformBuffersMapped;
public:
	explicit UniformBuffer() : maxFramesInFlight(Config::MAX_FRAMES_IN_FLIGHT) {}

	explicit
	UniformBuffer(const int maxFrames) : maxFramesInFlight(maxFrames) {}

	[[nodiscard]] auto GetMaxFramesInFlight() const -> const int& {
		return maxFramesInFlight;
	}

	[[nodiscard]] auto GetUniformBuffers() const -> const UniformBuffers& {
		return uniformBuffers;
	}

	auto CreateUniformBuffers(const Device& device) -> void;

	auto UpdateUniformBuffer(
		const VkExtent2D& swapChainExtent,
		uint32_t          currentImage
	) const -> void;

	auto Destroy(const VkDevice& device) const -> void;
};

class Descriptor {
	UniformBuffer         uniformBuffer {};
	VkDescriptorPool      descriptorPool {};
	VkDescriptorSetLayout descriptorSetLayout {};
	DescriptorSets        descriptorSets {};
public:
	[[nodiscard]] auto GetSetLayout() const -> const VkDescriptorSetLayout& {
		return descriptorSetLayout;
	}

	[[nodiscard]] auto GetDescriptorSets() const -> const DescriptorSets& {
		return descriptorSets;
	}

	auto CreateDescriptorSetLayout(const VkDevice& device) -> void;
	auto CreateDescriptorSets(const VkDevice& device) -> void;
	auto CreateDescriptorPool(const VkDevice& device) -> void;

	auto CreateUniformBuffers(const Device& device) -> void {
		uniformBuffer.CreateUniformBuffers(device);
	}

	auto UpdateUniformBuffers(
		const VkExtent2D& swapChainExtent,
		uint32_t          currentImage
	) const -> void {
		uniformBuffer.UpdateUniformBuffer(swapChainExtent, currentImage);
	}

	auto Destroy(const VkDevice& device) const -> void;
};
