#pragma once

#include "utils.h"

using MessengerCreateInfo = VkDebugUtilsMessengerCreateInfoEXT;

class Validation {
	const bool                enabled;
	const CStrings            layers;
	const MessengerCreateInfo messengerCreateInfo;

	VkDebugUtilsMessengerEXT debugMessenger {};
public:
	[[nodiscard]] auto GetEnabled() const -> const bool&;
	[[nodiscard]] auto GetLayers() const -> const CStrings&;

	Validation();
	Validation(
		bool                enabled,
		CStrings            layers,
		MessengerCreateInfo messengerInfo
	);

	[[nodiscard]] auto
	GetMessengerCreateInfo() const -> const MessengerCreateInfo&;
	[[nodiscard]] auto
	GetDebugMessenger() const -> const VkDebugUtilsMessengerEXT&;

	auto CreateMessengerEXT(
		const VkInstance&            instance,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT*    pDebugMessenger
	) const -> VkResult;

	static auto DestroyMessengerEXT(
		const VkInstance&               instance,
		const VkDebugUtilsMessengerEXT& debugMessenger,
		const VkAllocationCallbacks*    pAllocator
	) -> void;

	[[nodiscard]] auto CheckLayerSupport() const -> bool;

	auto SetupMessenger(const VkInstance& instance) -> void;
};
