#include "../include/uniform.h"

#include <chrono>
#include <stdexcept>

#include "../include/device.h"
#include "../include/render.h"
#include "../include/buffer.h"
#include "../include/texture.h"


void Descriptor::CreateDescriptorSets(const VkDevice& device,
	const Render& render,
	const VkDescriptorSetLayout& descriptorSetLayout,
	const std::vector<Texture>& textures) {
	const std::vector layouts(render.GetMaxFramesInFlight(),
		descriptorSetLayout);

	const VkDescriptorSetAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(render.
			GetMaxFramesInFlight()),
		.pSetLayouts = layouts.data(),
	};

	descriptorSets.resize(render.GetMaxFramesInFlight());
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) !=
	    VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets!");
	}

	for (auto i = 0; i < render.GetMaxFramesInFlight(); i++) {
		std::vector<VkWriteDescriptorSet> descriptorWrites;
		descriptorWrites.resize(textures.size() + 1);

		VkDescriptorBufferInfo bufferInfo {
			.buffer = uniformBuffer.GetUniformBuffers()[i],
			.offset = 0,
			.range = sizeof(UniformBufferObject),
		};
		descriptorWrites[0] = VkWriteDescriptorSet {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptorSets[i],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &bufferInfo,
		};

		std::vector<VkDescriptorImageInfo> imageInfos;
		imageInfos.resize(textures.size());

		for (size_t j = 0; j < textures.size(); j++) {
			imageInfos[j] = {
				.sampler = textures[j].GetTextureSampler(),
				.imageView = textures[j].GetTextureImageView(),
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			};
			descriptorWrites[j + 1] = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = descriptorSets[i],
				.dstBinding = static_cast<uint32_t>(j + 1),
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &imageInfos[j],
			};
		}
		vkUpdateDescriptorSets(device,
			static_cast<uint32_t>(descriptorWrites.size()),
			descriptorWrites.data(),
			0,
			nullptr);
	}
}

void Descriptor::CreateDescriptorPool(const VkDevice& device,
	const Render& render,
	const size_t textureNum) {
	std::vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.resize(textureNum + 1);

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(render.
		GetMaxFramesInFlight());

	for (size_t i = 0; i < textureNum; i++) {
		poolSizes[i + 1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[i + 1].descriptorCount = static_cast<uint32_t>(render.
			GetMaxFramesInFlight());
	}

	const VkDescriptorPoolCreateInfo poolInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = static_cast<uint32_t>(render.GetMaxFramesInFlight()),
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data(),
	};

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void Descriptor::Create(const Device& device,
	const Render& render,
	const VkDescriptorSetLayout& descriptorSetLayout,
	const std::vector<Texture>& textures) {
	CreateUniformBuffers(device, render);
	CreateDescriptorPool(device.GetLogical(), render, textures.size());
	CreateDescriptorSets(device.GetLogical(),
		render,
		descriptorSetLayout,
		textures);
}

void Descriptor::Destroy(const VkDevice& device, const Render& render) const {
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	uniformBuffer.Destroy(device, render);
}

void UniformBuffer::CreateUniformBuffers(const Device& device,
	const Render& render) {
	uniformBuffers.resize(render.GetMaxFramesInFlight());
	uniformBuffersMemory.resize(render.GetMaxFramesInFlight());
	uniformBuffersMapped.resize(render.GetMaxFramesInFlight());

	for (size_t i = 0; i < render.GetMaxFramesInFlight(); i++) {
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
	const auto time = std::chrono::duration<float>(currentTime - startTime).
		count();

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

void UniformBuffer::Destroy(const VkDevice& device,
	const Render& render) const {
	for (auto i = 0; i < render.GetMaxFramesInFlight(); i++) {
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}
}