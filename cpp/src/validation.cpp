#include <iostream>
#include <utility>
#include <vulkan/vulkan_core.h>

#include "validation.h"
#include "config.h"

auto VKAPI_CALL DebugCallback(
	[[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
	[[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	[[maybe_unused]] void* pUserData
) -> VkBool32 {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

auto Validation::GetEnabled() const -> const bool& { return enabled; }
auto Validation::GetLayers() const -> const CStrings& { return layers; }

auto Validation::GetMessengerCreateInfo() const -> const MessengerCreateInfo& {
	return messengerCreateInfo;
}

auto Validation::GetDebugMessenger() const -> const VkDebugUtilsMessengerEXT& {
	return debugMessenger;
}

Validation::Validation() : enabled(ENABLE_VALIDATION_LAYER),
	layers({ "VK_LAYER_KHRONOS_validation" }),
	messengerCreateInfo(
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = DebugCallback,
		}
	) {}

Validation::Validation(
	const bool          enabled,
	CStrings            layers,
	MessengerCreateInfo messengerInfo
) : enabled(enabled), layers(std::move(layers)),
	messengerCreateInfo(messengerInfo) {}

auto Validation::CreateMessengerEXT(
	const VkInstance&            instance,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT*    pDebugMessenger
) const -> VkResult {
	if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"))) {
		return func(
			instance,
			&messengerCreateInfo,
			pAllocator,
			pDebugMessenger
		);
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

auto Validation::DestroyMessengerEXT(
	const VkInstance&               instance,
	const VkDebugUtilsMessengerEXT& debugMessenger,
	const VkAllocationCallbacks*    pAllocator
) -> void {
	if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"))) {
		func(instance, debugMessenger, pAllocator);
	}
}

auto Validation::CheckLayerSupport() const -> bool {
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const auto& layerName: layers) {
		auto layerFound = false;
		for (const auto& [availableLayerName, specVersion, implementationVersion
			     , description]: availableLayers) {
			if (strcmp(layerName, availableLayerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) { return false; }
	}
	return true;
}

auto Validation::SetupMessenger(const VkInstance& instance) -> void {
	if (enabled && CreateMessengerEXT(instance, nullptr, &debugMessenger) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}
