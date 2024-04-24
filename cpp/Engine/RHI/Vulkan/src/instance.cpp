#include "../include/instance.h"

#include <stdexcept>
#include <vector>

#include "../include/validation.h"
#include "../include/window.h"

std::vector<const char*> GetRequiredExtensions(const Validation& validation) {
  const auto& [glfwExtensions, glfwExtensionCount] =
      Window::GetRequiredExtensions();
  std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (validation.GetEnabled()) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  return extensions;
}

void Instance::CreateInstance(const Validation& validation) {
  if (validation.GetEnabled() && !validation.CheckLayerSupport()) {
    PRINT_AND_THROW_ERROR("validation layers requested, but not available!");
  }
  const auto extensions = GetRequiredExtensions(validation);

  VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "Hello Triangle",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_3,
  };

  VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
  };

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (validation.GetEnabled()) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validation.GetLayers().size());
    createInfo.ppEnabledLayerNames = validation.GetLayers().data();

    debugCreateInfo = validation.GetMessengerCreateInfo();
    createInfo.pNext = &debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    PRINT_AND_THROW_ERROR("failed to create instance!");
  }
}

void Instance::DestroyInstance() const { vkDestroyInstance(instance, nullptr); }
