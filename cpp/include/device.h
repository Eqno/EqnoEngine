#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "validation.h"

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR        capabilities{};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>   presentModes;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	/**
	 * 当前查询是否完成
	 */
	[[nodiscard]] auto IsComplete() const -> bool {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

class Device {
	/** Private Members **/
	VkDevice         logicalDevice {};
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	VkQueue graphicsQueue {};
	VkQueue presentQueue {};
public:
	/** Behaviors And Logic **/
	/**
	 * 使当前逻辑设备闲置等待
	 */
	auto WaitIdle() const -> void { vkDeviceWaitIdle(logicalDevice); }

	/** Finders And Queries **/
	/**
	 * 查找当前物理设备的队列族
	 */
	[[nodiscard]] auto FindQueueFamilies(
		const VkSurfaceKHR& surface
	) const -> QueueFamilyIndices;
	/**
	 * 查询当前物理设备的交换链支持
	 */
	[[nodiscard]] auto QuerySwapChainSupport(
		const VkSurfaceKHR& surface
	) const -> SwapChainSupportDetails;

	/** Pickers And Creators **/
	/**
	 * 创建逻辑设备的实例
	 */
	auto CreateLogicalDevice(
		const VkSurfaceKHR& surface,
		const Validation&   validation
	) -> void;
	/**
	 * 拾取物理设备的实例
	 */
	auto PickPhysicalDevice(
		const VkInstance&   instance,
		const VkSurfaceKHR& surface
	) -> void;

	/** Getters And Setters **/
	/**
	 * 获取逻辑设备的引用
	 */
	[[nodiscard]] auto GetLogical() const -> const VkDevice& {
		return logicalDevice;
	}

	/**
	 * 获取物理设备的引用
	 */
	[[nodiscard]] auto GetPhysical() const -> const VkPhysicalDevice& {
		return physicalDevice;
	}

	/**
	 * 获取即时设备的队列
	 */
	[[nodiscard]] auto GetPresentQueue() const -> const VkQueue& {
		return presentQueue;
	}

	/**
	 * 获取图形设备的队列
	 */
	[[nodiscard]] auto GetGraphicsQueue() const -> const VkQueue& {
		return graphicsQueue;
	}
};