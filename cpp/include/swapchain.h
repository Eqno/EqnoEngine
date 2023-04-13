#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

using PresentModes = std::vector<VkPresentModeKHR>;
using SurfaceFormats = std::vector<VkSurfaceFormatKHR>;

class Window;
class Device;

class SwapChain {
	VkSwapchainKHR chain {};
	VkExtent2D     extent {};
	VkFormat       imageFormat {};

	std::vector<VkImage>       images;
	std::vector<VkImageView>   imageViews;
	std::vector<VkFramebuffer> frameBuffers;
public:
	static auto ChooseSurfaceFormat(
		const SurfaceFormats& availableFormats
	) -> VkSurfaceFormatKHR;

	static auto ChoosePresentMode(
		const PresentModes& availablePresentModes
	) -> VkPresentModeKHR;

	[[nodiscard]] auto ChooseSwapExtent(
		const VkSurfaceCapabilitiesKHR& capabilities,
		const Window&                   window
	) const -> VkExtent2D;

	[[nodiscard]] auto Get() const -> const VkSwapchainKHR& { return chain; }
	[[nodiscard]] auto GetExtent() const -> const VkExtent2D& { return extent; }

	[[nodiscard]] auto GetImageFormat() const -> const VkFormat& {
		return imageFormat;
	}

	[[nodiscard]] auto
	GetImageViews() const -> const std::vector<VkImageView>& {
		return imageViews;
	}

	[[nodiscard]] auto
	GetFrameBuffers() const -> const std::vector<VkFramebuffer>& {
		return frameBuffers;
	}

	auto Create(const Device& device, const Window& window) -> void;
	auto CreateImageViews(const VkDevice& device) -> void;
	auto CleanupSwapChain(const VkDevice& device) const -> void;
	auto CreateFrameBuffers(
		const VkDevice&     device,
		const VkRenderPass& renderPass
	) -> void;
};
