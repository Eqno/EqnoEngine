#include <Engine/RHI/Vulkan/include/config.h>
#include <Engine/RHI/Vulkan/include/device.h>
#include <Engine/RHI/Vulkan/include/validation.h>
#include <Engine/RHI/Vulkan/include/vulkan.h>
#include <Engine/Utility/include/TypeUtils.h>

#include <stdexcept>

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
QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& device,
                                     const VkSurfaceKHR& surface) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  QueueFamilyProps queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies.data());

  QueueFamilyIndices indices = {};
  for (auto i = 0; const auto& [queueFlags, queueCount, timestampValidBits,
                                minImageTransferGranularity] : queueFamilies) {
    if (queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
    if (presentSupport) {
      indices.presentFamily = i;
    }
    if (indices.IsComplete()) {
      break;
    }
    i++;
  }
  return indices;
}

/**
 * 检查某一设备是否支持所需的插件
 */
bool DoesExtensionsSupport(const VkPhysicalDevice& device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);
  ExtensionProps availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       availableExtensions.data());

  StringSet requiredExtensions(VulkanConfig::DEVICE_EXTENSIONS.begin(),
                               VulkanConfig::DEVICE_EXTENSIONS.end());
  for (const auto& [extensionName, _] : availableExtensions) {
    requiredExtensions.erase(extensionName);
  }
  return requiredExtensions.empty();
}

/**
 * 查询某一设备的交换链支持情况
 */
SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice& device,
                                              const VkSurfaceKHR& surface) {
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         details.formats.data());
  }
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                            nullptr);
  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentModeCount, details.presentModes.data());
  }
  return details;
}

/**
 * 查询某一设备是否满足 Config 内配置的要求
 */
bool DoesRequiresSuit(const VkPhysicalDevice& device,
                      const VkSurfaceKHR& surface) {
  if (FindQueueFamilies(device, surface).IsComplete() &&
      DoesExtensionsSupport(device)) {
    const auto& [_, formats, presentModes] =
        QuerySwapChainSupport(device, surface);
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    return !formats.empty() && !presentModes.empty() &&
           supportedFeatures.samplerAnisotropy;
  }
  return false;
}
}  // namespace DeviceCheck

SwapChainSupportDetails Device::QuerySwapChainSupport(
    const VkSurfaceKHR& surface) const {
  return DeviceCheck::QuerySwapChainSupport(physicalDevice, surface);
}

QueueFamilyIndices Device::FindQueueFamilies(
    const VkSurfaceKHR& surface) const {
  return DeviceCheck::FindQueueFamilies(physicalDevice, surface);
}

VkSampleCountFlagBits Device::GetMaxUsableSampleCount(int msaaMaxSamples) {
  VkPhysicalDeviceProperties physicalDeviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

  VkSampleCountFlags counts =
      physicalDeviceProperties.limits.framebufferColorSampleCounts &
      physicalDeviceProperties.limits.framebufferDepthSampleCounts;
  if (msaaMaxSamples >= 64 && (counts & VK_SAMPLE_COUNT_64_BIT)) {
    msaaSamplesNum = 64;
    return VK_SAMPLE_COUNT_64_BIT;
  }
  if (msaaMaxSamples >= 32 && (counts & VK_SAMPLE_COUNT_32_BIT)) {
    msaaSamplesNum = 32;
    return VK_SAMPLE_COUNT_32_BIT;
  }
  if (msaaMaxSamples >= 16 && (counts & VK_SAMPLE_COUNT_16_BIT)) {
    msaaSamplesNum = 16;
    return VK_SAMPLE_COUNT_16_BIT;
  }
  if (msaaMaxSamples >= 8 && (counts & VK_SAMPLE_COUNT_8_BIT)) {
    msaaSamplesNum = 8;
    return VK_SAMPLE_COUNT_8_BIT;
  }
  if (msaaMaxSamples >= 4 && (counts & VK_SAMPLE_COUNT_4_BIT)) {
    msaaSamplesNum = 4;
    return VK_SAMPLE_COUNT_4_BIT;
  }
  if (msaaMaxSamples >= 2 && (counts & VK_SAMPLE_COUNT_2_BIT)) {
    msaaSamplesNum = 2;
    return VK_SAMPLE_COUNT_2_BIT;
  }
  msaaSamplesNum = 1;
  return VK_SAMPLE_COUNT_1_BIT;
}

uint32_t Device::GetMinUniformBufferOffsetAlignment() {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  return properties.limits.minUniformBufferOffsetAlignment;
}

uint32_t Device::GetMultiSampleNum() const { return msaaSamplesNum; }
uint32_t Device::GetMinUBOOffsetAlignment() const {
  return minUBOOffsetAlignment;
}

void Device::PickPhysicalDevice(const VkInstance& instance,
                                const VkSurfaceKHR& surface) {
  uint32_t deviceCount = 0;
  if (vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) !=
          VK_SUCCESS ||
      deviceCount == 0) {
    PRINT_AND_THROW_ERROR("failed to find GPUs with Vulkan support!");
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (const auto& device : devices) {
    if (DeviceCheck::DoesRequiresSuit(device, surface)) {
      physicalDevice = device;
      msaaSamples = GetMaxUsableSampleCount(
          static_cast<Vulkan*>(owner)->GetMSAASamples());
      minUBOOffsetAlignment = GetMinUniformBufferOffsetAlignment();
      break;
    }
  }
  if (physicalDevice == VK_NULL_HANDLE) {
    PRINT_AND_THROW_ERROR("failed to find a suitable GPU!");
  }
}

void Device::DestroyLogicalDevice() const {
  vkDestroyDevice(logicalDevice, nullptr);
}

void Device::CreateLogicalDevice(const VkSurfaceKHR& surface,
                                 const Validation& validation) {
  constexpr auto queuePriorities = 1.0f;
  DeviceCheck::QueueCreateInfos queueCreateInfos{};

  const auto [_graphicsFamily, _presentFamily] =
      DeviceCheck::FindQueueFamilies(physicalDevice, surface);
  graphicsFamily = _graphicsFamily.has_value() ? _graphicsFamily.value() : 0;
  presentFamily = _presentFamily.has_value() ? _presentFamily.value() : 0;

  for (const std::set uniqueQueueFamilies = {graphicsFamily, presentFamily};
       const auto queueFamily : uniqueQueueFamilies) {
    queueCreateInfos.push_back({
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriorities,
    });
  }
  VkPhysicalDeviceFeatures deviceFeatures{.samplerAnisotropy = VK_TRUE};
  VkDeviceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
      .pQueueCreateInfos = queueCreateInfos.data(),
      .enabledExtensionCount =
          static_cast<uint32_t>(VulkanConfig::DEVICE_EXTENSIONS.size()),
      .ppEnabledExtensionNames = VulkanConfig::DEVICE_EXTENSIONS.data(),
      .pEnabledFeatures = &deviceFeatures,
  };
  if (validation.GetEnabled()) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validation.GetLayers().size());
    createInfo.ppEnabledLayerNames = validation.GetLayers().data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) !=
      VK_SUCCESS) {
    PRINT_AND_THROW_ERROR("failed to create logical logicalDevice!");
  }
  vkGetDeviceQueue(logicalDevice, graphicsFamily, 0, &graphicsQueue);
  vkGetDeviceQueue(logicalDevice, presentFamily, 0, &presentQueue);
}
