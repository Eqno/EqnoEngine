#include "validation.h"

#include <iostream>
#include <vulkan/vulkan_core.h>

#include "config.h"

VkBool32 VKAPI_CALL DebugCallback(
	[[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
	[[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	[[maybe_unused]] void* pUserData
) {
	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

Validation::Validation() : enabled(Config::ENABLE_VALIDATION_LAYER),
	layers({"VK_LAYER_KHRONOS_validation"}),
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
	const bool enabled,
	CStrings layers,
	const MessengerCreateInfo& messengerInfo
) : enabled(enabled), layers(std::move(layers)),
	messengerCreateInfo(messengerInfo) {}

VkResult Validation::CreateMessengerEXT(
	const VkInstance& instance,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger
) const {
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

void Validation::DestroyMessengerEXT(
	const VkInstance& instance,
	const VkDebugUtilsMessengerEXT& debugMessenger,
	const VkAllocationCallbacks* pAllocator
) {
	if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"))) {
		func(instance, debugMessenger, pAllocator);
	}
}

bool Validation::CheckLayerSupport() const {
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
		if (!layerFound) {
			return false;
		}
	}
	return true;
}

void Validation::SetupMessenger(const VkInstance& instance) {
	if (enabled && CreateMessengerEXT(instance, nullptr, &debugMessenger) !=
		VK_SUCCESS) {
		throw std::runtime_error("Failed to set up debug messenger!");
	}
}

void Validation::DestroyMessenger(const VkInstance& instance) const {
	if (enabled) {
		DestroyMessengerEXT(
			instance,
			debugMessenger,
			nullptr
		);
	}
}
