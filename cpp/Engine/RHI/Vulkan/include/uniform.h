#pragma once

#include <vulkan/vulkan_core.h>
#include "Engine/Utility/include/TypeUtils.h"

class Mesh;
class Device;
class Render;
class Texture;
class Descriptor;

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
	Descriptor* _owner;

	UniformBuffers uniformBuffers;
	UniformMemories uniformBuffersMemory;
	UniformMapped uniformBuffersMapped;

public:
	void RegisterOwner(Descriptor* other) {
		_owner = other;
	}
	explicit UniformBuffer() = default;

	[[nodiscard]] const UniformBuffers& GetUniformBuffers() const {
		return uniformBuffers;
	}

	[[nodiscard]] const VkBuffer& GetUniformBufferByIndex(
		const size_t index) const {
		return uniformBuffers[index];
	}

	void CreateUniformBuffers(const Device& device, const Render& render);

	void UpdateUniformBuffer(uint32_t currentImage) const;

	void Destroy(const VkDevice& device, const Render& render) const;
};

class Descriptor {
	Mesh* _owner;

	UniformBuffer uniformBuffer {};
	VkDescriptorPool descriptorPool {};
	DescriptorSets descriptorSets {};

	void CreateDescriptorSets(const VkDevice& device,
		const Render& render,
		const VkDescriptorSetLayout& descriptorSetLayout,
		const std::vector<Texture>& textures);

	void CreateDescriptorPool(const VkDevice& device,
		const Render& render,
		size_t textureNum);

	void CreateUniformBuffer(const Device& device, const Render& render);
	void DestroyUniformBuffers(const VkDevice& device,
		const Render& render) const;

public:
	void RegisterOwner(Mesh* other) {
		_owner = other;
	}

	void UpdateUniformBuffer(const uint32_t currentImage) const {
		uniformBuffer.UpdateUniformBuffer(currentImage);
	}

	[[nodiscard]] const UniformBuffer& GetUniformBuffer() const {
		return uniformBuffer;
	}

	[[nodiscard]] const UniformBuffers& GetUniformBuffers() const {
		return uniformBuffer.GetUniformBuffers();
	}

	[[nodiscard]] const VkBuffer& GetUniformBufferByIndex(
		const size_t index) const {
		return uniformBuffer.GetUniformBufferByIndex(index);
	}

	[[nodiscard]] const DescriptorSets& GetDescriptorSets() const {
		return descriptorSets;
	}

	[[nodiscard]] const VkDescriptorSet& GetDescriptorSetByIndex(
		const uint32_t index) const {
		return descriptorSets[index];
	}

	void Create(const Device& device,
		const Render& render,
		const VkDescriptorSetLayout& descriptorSetLayout,
		const std::vector<Texture>& textures);
	void Destroy(const VkDevice& device, const Render& render) const;

	// Update uniform buffer data
	const glm::mat4x4* GetModelMatrix();
	const glm::mat4x4* GetViewMatrix();
	const glm::mat4x4* GetProjMatrix();
};
