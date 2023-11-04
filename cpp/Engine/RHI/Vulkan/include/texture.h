#pragma once
#include <vulkan/vulkan_core.h>

#include "device.h"
#include "mesh.h"

class Render;

class Texture {
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

public:
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

	void CreateTextureImage(const Device& device, const Mesh& mesh, const Render& render);
	void CreateTextureImageView(const VkDevice& device);
	void CreateTextureSampler(const Device& device);

	static VkImageView CreateImageView(const VkDevice& device,
		const VkImage image,
		const VkFormat format,
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

	static void TransitionImageLayout(const Device& device,
		const Render& render,
		const VkImage image,
		const VkFormat format,
		const Mesh& mesh,
		const VkImageLayout oldLayout,
		const VkImageLayout newLayout);
	static void CopyBufferToImage(const Device& device,
		const Mesh& mesh,
		const Render& render,
		const VkBuffer buffer,
		const VkImage image,
		const uint32_t width,
		const uint32_t height);
	void Destroy(const VkDevice& device) const;
};
