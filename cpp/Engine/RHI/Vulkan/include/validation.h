#pragma once

#include <vulkan/vulkan_core.h>

#include "Engine/Utility/include/TypeUtils.h"
#include "base.h"

using MessengerCreateInfo = VkDebugUtilsMessengerCreateInfoEXT;

class Validation : public Base {
  bool enabled;
  CStrings layers;
  MessengerCreateInfo messengerCreateInfo;

  VkDebugUtilsMessengerEXT debugMessenger{};

  static void DestroyMessengerEXT(
      const VkInstance& instance,
      const VkDebugUtilsMessengerEXT& debugMessenger,
      const VkAllocationCallbacks* pAllocator);

  VkResult CreateMessengerEXT(const VkInstance& instance,
                              const VkAllocationCallbacks* pAllocator,
                              VkDebugUtilsMessengerEXT* pDebugMessenger) const;

 public:
  [[nodiscard]] const bool& GetEnabled() const { return enabled; }

  [[nodiscard]] const CStrings& GetLayers() const { return layers; }

  Validation();
  Validation(bool enabled, CStrings layers,
             const MessengerCreateInfo& messengerInfo);

  [[nodiscard]] const MessengerCreateInfo& GetMessengerCreateInfo() const {
    return messengerCreateInfo;
  }

  [[nodiscard]] const VkDebugUtilsMessengerEXT& GetDebugMessenger() const {
    return debugMessenger;
  }

  [[nodiscard]] bool CheckLayerSupport() const;

  void SetupMessenger(const VkInstance& instance);
  void DestroyMessenger(const VkInstance& instance) const;
};
