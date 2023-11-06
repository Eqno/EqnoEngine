#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "utils.h"

using PresentModes = std::vector<VkPresentModeKHR>;
using SurfaceFormats = std::vector<VkSurfaceFormatKHR>;

class Depth;
class Window;
class Device;

class SwapChain {
	VkSwapchainKHR chain {};
	VkExtent2D extent {};
	VkFormat imageFormat {};

	VkFormat surfaceFormat = VK_FORMAT_R8G8B8A8_SRGB;
	VkColorSpaceKHR surfaceColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	std::vector<VkFramebuffer> frameBuffers;

public:
	[[nodiscard]] VkSurfaceFormatKHR ChooseSurfaceFormat(
		const SurfaceFormats& availableFormats) const;

	static VkPresentModeKHR ChoosePresentMode(
		const PresentModes& availablePresentModes);

	[[nodiscard]] static VkExtent2D ChooseSwapExtent(
		const VkSurfaceCapabilitiesKHR& capabilities,
		const Window& window);

	[[nodiscard]] const VkSwapchainKHR& Get() const {
		return chain;
	}

	[[nodiscard]] const VkExtent2D& GetExtent() const {
		return extent;
	}

	[[nodiscard]] const VkFormat& GetImageFormat() const {
		return imageFormat;
	}

	[[nodiscard]] const std::vector<VkImageView>& GetImageViews() const {
		return imageViews;
	}

	[[nodiscard]] const std::vector<VkFramebuffer>& GetFrameBuffers() const {
		return frameBuffers;
	}

	void CreateImageViews(const VkDevice& device);
	void CleanupSwapChain(const VkDevice& device, const Depth& depth) const;
	void CreateFrameBuffers(const VkDevice& device,
		const Depth& depth,
		const VkRenderPass& renderPass);
	void RecreateSwapChain(const Device& device,
		Depth& depth,
		const Window& window,
		const VkRenderPass& renderPass);

	void Create(const Device& device, const Window& window);

	void Create(const std::string& format,
		const Device& device,
		const Window& window);

	void Create(const std::string& format,
		const std::string& space,
		const Device& device,
		const Window& window);

	void CreateSwapChain(const Device& device, const Window& window);
};
