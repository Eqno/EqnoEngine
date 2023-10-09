#include "uniform.h"

#include <chrono>
#include <stdexcept>

#include "buffer.h"

void Descriptor::CreateDescriptorSetLayout(const VkDevice& device) {
	constexpr VkDescriptorSetLayoutBinding uboLayoutBinding {
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr,
	};

	constexpr VkDescriptorSetLayoutCreateInfo layoutInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &uboLayoutBinding,
	};

	if (vkCreateDescriptorSetLayout(
		device,
		&layoutInfo,
		nullptr,
		&descriptorSetLayout
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}

void Descriptor::CreateDescriptorSets(const VkDevice& device) {
	const std::vector layouts(
		uniformBuffer.GetMaxFramesInFlight(),
		descriptorSetLayout
	);

	const VkDescriptorSetAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(uniformBuffer.GetMaxFramesInFlight()),
		.pSetLayouts = layouts.data(),
	};

	descriptorSets.resize(uniformBuffer.GetMaxFramesInFlight());
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) !=
		VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets!");
	}

	for (auto i = 0; i < uniformBuffer.GetMaxFramesInFlight(); i++) {
		VkDescriptorBufferInfo bufferInfo {
			.buffer = uniformBuffer.GetUniformBuffers()[i],
			.offset = 0,
			.range = sizeof(UniformBufferObject),
		};

		VkWriteDescriptorSet descriptorWrite {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptorSets[i],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &bufferInfo,
		};

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
	}
}

void Descriptor::CreateDescriptorPool(const VkDevice& device) {
	constexpr VkDescriptorPoolSize poolSize {
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = static_cast<uint32_t>(Config::MAX_FRAMES_IN_FLIGHT),
	};

	constexpr VkDescriptorPoolCreateInfo poolInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = static_cast<uint32_t>(Config::MAX_FRAMES_IN_FLIGHT),
		.poolSizeCount = 1,
		.pPoolSizes = &poolSize,
	};

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void Descriptor::Destroy(const VkDevice& device) const {
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	uniformBuffer.Destroy(device);
}

void UniformBuffer::CreateUniformBuffers(const Device& device) {
	uniformBuffers.resize(Config::MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(Config::MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(Config::MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < Config::MAX_FRAMES_IN_FLIGHT; i++) {
		Buffer::CreateBuffer(
			device,
			sizeof(UniformBufferObject),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffers[i],
			uniformBuffersMemory[i]
		);

		vkMapMemory(
			device.GetLogical(),
			uniformBuffersMemory[i],
			0,
			sizeof(UniformBufferObject),
			0,
			&uniformBuffersMapped[i]
		);
	}
}

void UniformBuffer::UpdateUniformBuffer(
	const VkExtent2D& swapChainExtent,
	const uint32_t currentImage) const {
	static auto startTime = std::chrono::high_resolution_clock::now();
	const auto currentTime = std::chrono::high_resolution_clock::now();

	const auto time = std::chrono::duration<float, std::chrono::seconds::period>(
		currentTime - startTime
	).count();

	UniformBufferObject ubo {
		.model = rotate(
			glm::mat4(1.0f),
			time * glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		),
		.view = lookAt(
			glm::vec3(2.0f, 2.0f, 2.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		),
		.proj = glm::perspective(
			glm::radians(45.0f),
			static_cast<float>(swapChainExtent.width)
			/ static_cast<float>(swapChainExtent.height),
			0.1f,
			10.0f
		),
	};
	ubo.proj[1][1] *= -1;
	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void UniformBuffer::Destroy(const VkDevice& device) const {
	for (auto i = 0; i < maxFramesInFlight; i++) {
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}
}
