#pragma once
#include <vulkan/vulkan_core.h>

#include "device.h"
#include "data.h"

class Render;

class Texture {
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	void CreateTextureImage(const Device& device,
		const Render& render,
		const char* texturePath);
	void CreateTextureImageView(const VkDevice& device);
	void CreateTextureSampler(const Device& device);

	static void TransitionImageLayout(const Device& device,
		const Render& render,
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout);
	static void CopyBufferToImage(const Device& device,
		const Render& render,
		VkBuffer buffer,
		VkImage image,
		uint32_t width,
		uint32_t height);

public:
	Texture(const Device& device,
		const Render& render,
		const char* texturePath) : textureImage(nullptr),
	textureImageMemory(nullptr),
	textureImageView(nullptr),
	textureSampler(nullptr) {
		Create(device, render, texturePath);
	}

	Texture() : textureImage(nullptr),
	textureImageMemory(nullptr),
	textureImageView(nullptr),
	textureSampler(nullptr) {}

	Texture(const VkImage textureImage,
		const VkDeviceMemory textureImageMemory,
		const VkImageView textureImageView,
		const VkSampler textureSampler) : textureImage(textureImage),
	textureImageMemory(textureImageMemory),
	textureImageView(textureImageView),
	textureSampler(textureSampler) {}

	[[nodiscard]] const VkImageView& GetTextureImageView() const {
		return textureImageView;
	}

	[[nodiscard]] VkSampler GetTextureSampler() const {
		return textureSampler;
	}

	static VkImageView CreateImageView(const VkDevice& device,
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspectFlags);
	static void CreateImage(const Device& device,
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory);

	void Create(const Device& device,
		const Render& render,
		const char* texturePath);
	void Destroy(const VkDevice& device) const;
};
