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
	 * ��ǰ��ѯ�Ƿ����
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
	 * ʹ��ǰ�߼��豸���õȴ�
	 */
	auto WaitIdle() const -> void { vkDeviceWaitIdle(logicalDevice); }

	/** Finders And Queries **/
	/**
	 * ���ҵ�ǰ�����豸�Ķ�����
	 */
	[[nodiscard]] auto FindQueueFamilies(
		const VkSurfaceKHR& surface
	) const -> QueueFamilyIndices;
	/**
	 * ��ѯ��ǰ�����豸�Ľ�����֧��
	 */
	[[nodiscard]] auto QuerySwapChainSupport(
		const VkSurfaceKHR& surface
	) const -> SwapChainSupportDetails;

	/** Pickers And Creators **/
	/**
	 * �����߼��豸��ʵ��
	 */
	auto CreateLogicalDevice(
		const VkSurfaceKHR& surface,
		const Validation&   validation
	) -> void;
	/**
	 * ʰȡ�����豸��ʵ��
	 */
	auto PickPhysicalDevice(
		const VkInstance&   instance,
		const VkSurfaceKHR& surface
	) -> void;

	/** Getters And Setters **/
	/**
	 * ��ȡ�߼��豸������
	 */
	[[nodiscard]] auto GetLogical() const -> const VkDevice& {
		return logicalDevice;
	}

	/**
	 * ��ȡ�����豸������
	 */
	[[nodiscard]] auto GetPhysical() const -> const VkPhysicalDevice& {
		return physicalDevice;
	}

	/**
	 * ��ȡ��ʱ�豸�Ķ���
	 */
	[[nodiscard]] auto GetPresentQueue() const -> const VkQueue& {
		return presentQueue;
	}

	/**
	 * ��ȡͼ���豸�Ķ���
	 */
	[[nodiscard]] auto GetGraphicsQueue() const -> const VkQueue& {
		return graphicsQueue;
	}
};