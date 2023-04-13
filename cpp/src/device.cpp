#include <stdexcept>

#include "device.h"
#include "validation.h"

auto Device::FindQueueFamilies(
	const VkPhysicalDevice& device,
	const VkSurfaceKHR&     surface
) -> QueueFamilyIndices {
	QueueFamilyIndices indices;
	uint32_t           queueFamilyCount = 0;
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
	for (auto i = 0; const auto& [queueFlags, queueCount, timestampValidBits,
		     minImageTransferGranularity]: queueFamilies) {
		if (queueFlags & VK_QUEUE_GRAPHICS_BIT) { indices.graphicsFamily = i; }
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

auto Device::CheckExtensionSupport(const VkPhysicalDevice& device) -> bool {
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
	StringSet requiredExtensions(Extensions.begin(), Extensions.end());
	for (const auto& extension: availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();
}

auto Device::QuerySwapChainSupport(
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

auto Device::IsDeviceSuitable(
	const VkPhysicalDevice& device,
	const VkSurfaceKHR&     surface
) -> bool {
	if (FindQueueFamilies(device, surface).IsComplete() &&
		CheckExtensionSupport(device)) {
		auto [_, formats, presentModes] =
			QuerySwapChainSupport(device, surface);
		return ! formats.empty() && ! presentModes.empty();
	}
	return false;
}

auto Device::QuerySwapChainSupport(
	const VkSurfaceKHR& surface
) const -> SwapChainSupportDetails {
	return QuerySwapChainSupport(physicalDevice, surface);
}

auto Device::FindQueueFamilies(
	const VkSurfaceKHR& surface
) const -> QueueFamilyIndices {
	return FindQueueFamilies(physicalDevice, surface);
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
	for (const auto& dev: devices) {
		if (IsDeviceSuitable(dev, surface)) {
			physicalDevice = dev;
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
	const auto [graphicsFamily, presentFamily] = FindQueueFamilies(
		physicalDevice,
		surface
	);
	constexpr auto   queuePriorities = 1.0f;
	QueueCreateInfos queueCreateInfos;
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
		.enabledExtensionCount = static_cast<uint32_t>(Extensions.size()),
		.ppEnabledExtensionNames = Extensions.data(),
		.pEnabledFeatures = {},
	};
	if (validation.GetEnabled()) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validation.
			GetLayers().size());
		createInfo.ppEnabledLayerNames = validation.GetLayers().data();
	} else { createInfo.enabledLayerCount = 0; }
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}
	vkGetDeviceQueue(device, graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, presentFamily.value(), 0, &presentQueue);
}
