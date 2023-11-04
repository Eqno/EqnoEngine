#include "../include/uniform.h"

#include <chrono>
#include <array>
#include <stdexcept>

#include "../include/buffer.h"
#include "../include/texture.h"

void Descriptor::CreateDescriptorSetLayout(const VkDevice& device) {
	constexpr VkDescriptorSetLayoutBinding uboLayoutBinding {
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr,
	};

	constexpr VkDescriptorSetLayoutBinding samplerLayoutBinding {
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr,
	};

	std::array bindings = {uboLayoutBinding, samplerLayoutBinding};

	constexpr VkDescriptorSetLayoutCreateInfo layoutInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data(),
	};

	if (vkCreateDescriptorSetLayout(device,
		&layoutInfo,
		nullptr,
		&descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}

void Descriptor::CreateDescriptorSets(const VkDevice& device,
	const Texture& texture) {
	const std::vector layouts(uniformBuffer.GetMaxFramesInFlight(),
		descriptorSetLayout);

	const VkDescriptorSetAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(uniformBuffer.
			GetMaxFramesInFlight()),
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

		VkDescriptorImageInfo imageInfo {
			.sampler = texture.GetTextureSampler(),
			.imageView = texture.GetTextureImageView(),
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};

		std::array descriptorWrites {
			VkWriteDescriptorSet {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = descriptorSets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pBufferInfo = &bufferInfo,
			},
			VkWriteDescriptorSet {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = descriptorSets[i],
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &imageInfo,
			},
		};

		vkUpdateDescriptorSets(device,
			descriptorWrites.size(),
			descriptorWrites.data(),
			0,
			nullptr);
	}
}

void Descriptor::CreateDescriptorPool(const VkDevice& device) {
	std::array poolSizes {
		VkDescriptorPoolSize {
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = static_cast<uint32_t>(uniformBuffer.
				GetMaxFramesInFlight()),
		},
		VkDescriptorPoolSize {
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = static_cast<uint32_t>(uniformBuffer.
				GetMaxFramesInFlight()),
		},
	};

	VkDescriptorPoolCreateInfo poolInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = static_cast<uint32_t>(uniformBuffer.GetMaxFramesInFlight()),
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data(),
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
	uniformBuffers.resize(maxFramesInFlight);
	uniformBuffersMemory.resize(maxFramesInFlight);
	uniformBuffersMapped.resize(maxFramesInFlight);

	for (size_t i = 0; i < maxFramesInFlight; i++) {
		Buffer::CreateBuffer(device,
			sizeof(UniformBufferObject),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffers[i],
			uniformBuffersMemory[i]);

		vkMapMemory(device.GetLogical(),
			uniformBuffersMemory[i],
			0,
			sizeof(UniformBufferObject),
			0,
			&uniformBuffersMapped[i]);
	}
}

void UniformBuffer::UpdateUniformBuffer(const VkExtent2D& swapChainExtent,
	const uint32_t currentImage) const {
	static auto startTime = std::chrono::high_resolution_clock::now();
	const auto currentTime = std::chrono::high_resolution_clock::now();
	const auto time = std::chrono::duration<float>(currentTime - startTime).count();

	UniformBufferObject ubo {
		.model =
		rotate(glm::mat4(1.0f),
			time * glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)),
		.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)),
		.proj = glm::perspective(glm::radians(45.0f),
			static_cast<float>(swapChainExtent.width) / static_cast<float>(
				swapChainExtent.height),
			0.1f,
			10.0f),
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
