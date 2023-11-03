#pragma once
#include <inttypes.h>
#include <vulkan/vulkan_core.h>

#include "device.h"
#include "render.h"

class Texture {
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

public:
	Texture() : textureImage(), textureImageMemory() {}

	Texture(const VkImage textureImage, const VkDeviceMemory textureImageMemory) : textureImage(textureImage),
		textureImageMemory(textureImageMemory) {}

	void CreateTextureImage(const Device& device, const Render& render);

	static void CreateImage(const Device& device, uint32_t width, uint32_t height, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
		VkDeviceMemory& imageMemory);
	static void TransitionImageLayout(const Device& device, const Render& render, VkImage image, VkFormat format,
		VkImageLayout oldLayout, VkImageLayout newLayout);
	static void CopyBufferToImage(const Device& device, const Render& render, VkBuffer buffer, VkImage image,
		uint32_t width, uint32_t height);
	void Destroy(const VkDevice& device) const;
};
