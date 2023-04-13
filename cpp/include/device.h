#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

class Validation;

using ExtensionProps = std::vector<VkExtensionProperties>;
using QueueCreateInfos = std::vector<VkDeviceQueueCreateInfo>;
using QueueFamilyProps = std::vector<VkQueueFamilyProperties>;

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR        capabilities {};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>   presentModes;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	[[nodiscard]] auto IsComplete() const -> bool {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

class Device {
	VkDevice         device {};
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	VkQueue graphicsQueue {};
	VkQueue presentQueue {};
public:
	inline static const std::vector Extensions {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	auto WaitIdle() const -> void { vkDeviceWaitIdle(device); }

	auto PickPhysicalDevice(
		const VkInstance&   instance,
		const VkSurfaceKHR& surface
	) -> void;

	auto CreateLogicalDevice(
		const VkSurfaceKHR& surface,
		const Validation&   validation
	) -> void;

	[[nodiscard]] auto Get() const -> const VkDevice& { return device; }

	[[nodiscard]] auto GetPhysical() const -> const VkPhysicalDevice& {
		return physicalDevice;
	}

	[[nodiscard]] auto GetPresentQueue() const -> const VkQueue& {
		return presentQueue;
	}

	[[nodiscard]] auto GetGraphicsQueue() const -> const VkQueue& {
		return graphicsQueue;
	}

	static auto FindQueueFamilies(
		const VkPhysicalDevice& device,
		const VkSurfaceKHR&     surface
	) -> QueueFamilyIndices;

	static auto CheckExtensionSupport(const VkPhysicalDevice& device) -> bool;

	static auto QuerySwapChainSupport(
		const VkPhysicalDevice& device,
		const VkSurfaceKHR&     surface
	) -> SwapChainSupportDetails;

	static auto IsDeviceSuitable(
		const VkPhysicalDevice& device,
		const VkSurfaceKHR&     surface
	) -> bool;

	[[nodiscard]] auto QuerySwapChainSupport(
		const VkSurfaceKHR& surface
	) const -> SwapChainSupportDetails;

	[[nodiscard]] auto FindQueueFamilies(
		const VkSurfaceKHR& surface
	) const -> QueueFamilyIndices;
};
