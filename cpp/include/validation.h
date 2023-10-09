#pragma once

#include <vulkan/vulkan_core.h>

#include "utils.h"

using MessengerCreateInfo = VkDebugUtilsMessengerCreateInfoEXT;

class Validation {
	const bool enabled;
	const CStrings layers;
	const MessengerCreateInfo messengerCreateInfo;

	VkDebugUtilsMessengerEXT debugMessenger {};

public:
	[[nodiscard]] const bool& GetEnabled() const;
	[[nodiscard]] const CStrings& GetLayers() const;

	Validation();
	Validation(
		bool enabled,
		CStrings layers,
		const MessengerCreateInfo& messengerInfo
	);

	[[nodiscard]] const MessengerCreateInfo
	& GetMessengerCreateInfo() const;
	[[nodiscard]] const VkDebugUtilsMessengerEXT
	& GetDebugMessenger() const;

	VkResult CreateMessengerEXT(
		const VkInstance& instance,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger
	) const;

	static void DestroyMessengerEXT(
		const VkInstance& instance,
		const VkDebugUtilsMessengerEXT& debugMessenger,
		const VkAllocationCallbacks* pAllocator
	);

	[[nodiscard]] bool CheckLayerSupport() const;

	void SetupMessenger(const VkInstance& instance);
};
