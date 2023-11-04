#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "device.h"

class SwapChain;

class Depth {
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

public:
	Depth() : depthImage(nullptr),
	depthImageMemory(nullptr),
	depthImageView(nullptr) {}

	Depth(const VkImage depthImage,
		const VkDeviceMemory depthImageMemory,
		const VkImageView depthImageView) : depthImage(depthImage),
	depthImageMemory(depthImageMemory),
	depthImageView(depthImageView) {}

	[[nodiscard]] const VkImageView& GetDepthImageView() const {
		return depthImageView;
	}

	void CreateDepthResources(const Device& device,
		const VkExtent2D& swapChainExtent);
	static VkFormat FindSupportedFormat(const VkPhysicalDevice& device,
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features);
	static VkFormat FindDepthFormat(const VkPhysicalDevice& device);
	static bool HasStencilComponent(VkFormat format);
	void Destroy(const VkDevice& device) const;
};
