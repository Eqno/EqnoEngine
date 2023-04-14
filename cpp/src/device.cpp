#include "device.h"

#include <stdexcept>

#include "config.h"

/**
 * 设备检查相关函数
 */
namespace DeviceCheck {

	/** Type Alias **/
	using ExtensionProps = std::vector<VkExtensionProperties>;
	using QueueCreateInfos = std::vector<VkDeviceQueueCreateInfo>;
	using QueueFamilyProps = std::vector<VkQueueFamilyProperties>;

	/**
	 * 查找某一设备的队列组
	 */
	auto FindQueueFamilies(
		const VkPhysicalDevice& device,
		const VkSurfaceKHR&     surface
	) -> QueueFamilyIndices {

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(
			device,
			&queueFamilyCount,
			nullptr
		);
		QueueFamilyProps queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(
			device,
			&queueFamilyCount,
			queueFamilies.data()
		);

		QueueFamilyIndices indices = {};
		for (auto i = 0; const auto& [queueFlags, _, __, ___]: queueFamilies) {
			if (queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(
				device,
				i,
				surface,
				&presentSupport
			);
			if (presentSupport) { indices.presentFamily = i; }
			if (indices.IsComplete()) { break; }
			i++;
		}
		return indices;
	}

	/**
	 * 检查某一设备是否支持所需的插件
	 */
	auto DoesExtensionsSupport(const VkPhysicalDevice& device) -> bool {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(
			device,
			nullptr,
			&extensionCount,
			nullptr
		);
		ExtensionProps availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(
			device,
			nullptr,
			&extensionCount,
			availableExtensions.data()
		);

		StringSet requiredExtensions(
			Config::DEVICE_EXTENSIONS.begin(),
			Config::DEVICE_EXTENSIONS.end()
		);
		for (const auto& [extensionName, _]: availableExtensions) {
			requiredExtensions.erase(extensionName);
		}
		return requiredExtensions.empty();
	}

	/**
	 * 查询某一设备的交换链支持情况
	 */
	auto QuerySwapChainSupport(
		const VkPhysicalDevice& device,
		const VkSurfaceKHR&     surface
	) -> SwapChainSupportDetails {

		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			device,
			surface,
			&details.capabilities
		);
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device,
			surface,
			&formatCount,
			nullptr
		);
		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				device,
				surface,
				&formatCount,
				details.formats.data()
			);
		}
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device,
			surface,
			&presentModeCount,
			nullptr
		);
		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device,
				surface,
				&presentModeCount,
				details.presentModes.data()
			);
		}
		return details;
	}

	/**
	 * 查询某一设备是否满足 Config 内配置的要求
	 */
	auto DoesRequiresSuit(
		const VkPhysicalDevice& device,
		const VkSurfaceKHR&     surface
	) -> bool {
		if (FindQueueFamilies(device, surface).IsComplete() &&
			DoesExtensionsSupport(device)) {
			const auto& [_, formats, presentModes] = QuerySwapChainSupport(
				device,
				surface
			);
			return !formats.empty() && !presentModes.empty();
		}
		return false;
	}
}

auto Device::QuerySwapChainSupport(
	const VkSurfaceKHR& surface
) const -> SwapChainSupportDetails {
	return DeviceCheck::QuerySwapChainSupport(physicalDevice, surface);
}

auto Device::FindQueueFamilies(
	const VkSurfaceKHR& surface
) const -> QueueFamilyIndices {
	return DeviceCheck::FindQueueFamilies(physicalDevice, surface);
}

auto Device::PickPhysicalDevice(
	const VkInstance&   instance,
	const VkSurfaceKHR& surface
) -> void {

	uint32_t deviceCount = 0;
	if (vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) !=
		VK_SUCCESS || deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& device: devices) {
		if (DeviceCheck::DoesRequiresSuit(device, surface)) {
			physicalDevice = device;
			break;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

auto Device::CreateLogicalDevice(
	const VkSurfaceKHR& surface,
	const Validation&   validation
) -> void {
	constexpr auto                queuePriorities = 1.0f;
	DeviceCheck::QueueCreateInfos queueCreateInfos {};

	const auto [graphicsFamily, presentFamily] = DeviceCheck::FindQueueFamilies(
		physicalDevice,
		surface
	);
	for (const std::set uniqueQueueFamilies = {
		     graphicsFamily.value(),
		     presentFamily.value()
	     }; auto queueFamily: uniqueQueueFamilies) {
		queueCreateInfos.push_back(
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queueFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriorities,
			}
		);
	}
	VkDeviceCreateInfo createInfo {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledExtensionCount = static_cast<uint32_t>(Config::DEVICE_EXTENSIONS
			.size()),
		.ppEnabledExtensionNames = Config::DEVICE_EXTENSIONS.data(),
		.pEnabledFeatures = {},
	};
	if (validation.GetEnabled()) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validation.
			GetLayers().size());
		createInfo.ppEnabledLayerNames = validation.GetLayers().data();
	} else { createInfo.enabledLayerCount = 0; }

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create logical logicalDevice!");
	}
	vkGetDeviceQueue(logicalDevice, graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, presentFamily.value(), 0, &presentQueue);
}
