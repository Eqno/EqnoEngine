#include "uniform.h"

#include <chrono>
#include <stdexcept>

#include "buffer.h"

auto Descriptor::CreateDescriptorSetLayout(const VkDevice& device) -> void {
	VkDescriptorSetLayoutBinding uboLayoutBinding {};
	uboLayoutBinding.binding            = 0;
	uboLayoutBinding.descriptorCount    = 1;
	uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(
		device,
		&layoutInfo,
		nullptr,
		&descriptorSetLayout
	) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

auto Descriptor::CreateDescriptorSets(const VkDevice& device) -> void {
	const std::vector layouts(
		uniformBuffer.GetMaxFramesInFlight(),
		descriptorSetLayout
	);
	VkDescriptorSetAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(uniformBuffer.
		GetMaxFramesInFlight());
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(uniformBuffer.GetMaxFramesInFlight());
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < uniformBuffer.GetMaxFramesInFlight(); i++) {
		VkDescriptorBufferInfo bufferInfo {};
		bufferInfo.buffer = uniformBuffer.GetUniformBuffers()[i];
		bufferInfo.offset = 0;
		bufferInfo.range  = sizeof(UniformBufferObject);

		VkWriteDescriptorSet descriptorWrite {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
	}
}

auto Descriptor::CreateDescriptorPool(const VkDevice& device) -> void {
	VkDescriptorPoolSize poolSize {};
	poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(Config::MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo {};
	poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes    = &poolSize;
	poolInfo.maxSets       = static_cast<uint32_t>(Config::MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

auto Descriptor::Destroy(const VkDevice& device) const -> void {
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	uniformBuffer.Destroy(device);
}

auto UniformBuffer::CreateUniformBuffers(const Device& device) -> void {
	auto bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(Config::MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(Config::MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(Config::MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < Config::MAX_FRAMES_IN_FLIGHT; i++) {
		Buffer::CreateBuffer(
			device,
			bufferSize,
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
			bufferSize,
			0,
			&uniformBuffersMapped[i]
		);
	}
}

auto UniformBuffer::UpdateUniformBuffer(
	const VkExtent2D& swapChainExtent,
	uint32_t currentImage) const -> void {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration<float, std::chrono::seconds::period>(
		currentTime - startTime
		).count();

	UniformBufferObject ubo{};
	ubo.model = rotate(
		glm::mat4(1.0f),
		time * glm::radians(90.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	ubo.view = lookAt(
		glm::vec3(2.0f, 2.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	ubo.proj = glm::perspective(
		glm::radians(45.0f),
		swapChainExtent.width / static_cast<float>(swapChainExtent.height),
		0.1f,
		10.0f
	);
	ubo.proj[1][1] *= -1;

	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

auto UniformBuffer::Destroy(const VkDevice& device) const -> void {
	for (size_t i = 0; i < maxFramesInFlight; i++) {
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}
}
