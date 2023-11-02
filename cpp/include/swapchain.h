#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "pipeline.h"

using PresentModes = std::vector<VkPresentModeKHR>;
using SurfaceFormats = std::vector<VkSurfaceFormatKHR>;

class Window;
class Device;

class SwapChain {
	VkSwapchainKHR chain {};
	VkExtent2D extent {};
	VkFormat imageFormat {};

	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	std::vector<VkFramebuffer> frameBuffers;

public:
	static VkSurfaceFormatKHR ChooseSurfaceFormat(
		const SurfaceFormats& availableFormats
	);

	static VkPresentModeKHR ChoosePresentMode(
		const PresentModes& availablePresentModes
	);

	[[nodiscard]] static VkExtent2D ChooseSwapExtent(
		const VkSurfaceCapabilitiesKHR& capabilities,
		const Window& window
	);

	[[nodiscard]] const VkSwapchainKHR& Get() const {
		return chain;
	}

	[[nodiscard]] const VkExtent2D& GetExtent() const {
		return extent;
	}

	[[nodiscard]] const VkFormat& GetImageFormat() const {
		return imageFormat;
	}

	[[nodiscard]] const std::vector<VkImageView>
	& GetImageViews() const {
		return imageViews;
	}

	[[nodiscard]] const std::vector<VkFramebuffer>
	& GetFrameBuffers() const {
		return frameBuffers;
	}

	void Create(const Device& device, const Window& window);
	void CreateImageViews(const VkDevice& device);
	void CreateFrameBuffers(
		const VkDevice& device,
		const VkRenderPass& renderPass
	);

	void CleanupSwapChain(const VkDevice& device) const;
	void RecreateSwapChain(const Device& device, const Window& window, const Pipeline& pipeline);
};
