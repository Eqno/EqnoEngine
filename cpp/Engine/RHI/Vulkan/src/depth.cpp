#include "depth.h"

#include <stdexcept>
#include <vector>

#include "swapchain.h"
#include "texture.h"

void Depth::CreateDepthResources(const Device& device,
	const VkExtent2D& swapChainExtent) {
	const VkFormat depthFormat = FindDepthFormat(device.GetPhysical());
	Texture::CreateImage(device,
		swapChainExtent.width,
		swapChainExtent.height,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImage,
		depthImageMemory);
	depthImageView = Texture::CreateImageView(device.GetLogical(),
		depthImage,
		depthFormat,
		VK_IMAGE_ASPECT_DEPTH_BIT);
}

VkFormat Depth::FindSupportedFormat(const VkPhysicalDevice& device,
	const std::vector<VkFormat>& candidates,
	const VkImageTiling tiling,
	const VkFormatFeatureFlags features) {
	for (const VkFormat format: candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(device, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures &
		                                         features) == features) {
			return format;
		}
		if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures &
		                                          features) == features) {
			return format;
		}
	}
	throw std::runtime_error("failed to find supported format!");
}

VkFormat Depth::FindDepthFormat(const VkPhysicalDevice& device) {
	return FindSupportedFormat(device,
		{
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool Depth::HasStencilComponent(const VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format ==
	       VK_FORMAT_D24_UNORM_S8_UINT;
}

void Depth::Destroy(const VkDevice& device) const {
	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);
}
