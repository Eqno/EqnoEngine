#pragma once

#include <vulkan/vulkan_core.h>

#include <optional>
#include <vector>

#include "base.h"

class Validation;

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities{};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  /**
   * 当前查询是否完成
   */
  [[nodiscard]] bool IsComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

class Device : public Base {
  /** Private Members **/
  VkDevice logicalDevice;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  uint32_t graphicsFamily;
  uint32_t presentFamily;
  VkQueue graphicsQueue;
  VkQueue presentQueue;

  uint32_t msaaSamplesNum = 1;
  VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
  uint32_t minUBOOffsetAlignment = 0;

  VkSampleCountFlagBits GetMaxUsableSampleCount(int msaaMaxSamples);
  uint32_t GetMinUniformBufferOffsetAlignment();

 public:
  uint32_t GetMultiSampleNum() const;
  uint32_t GetMinUBOOffsetAlignment() const;

  uint32_t GetGraphicsFamily() const { return graphicsFamily; }
  uint32_t GetPresentFamily() const { return presentFamily; }

  /** Behaviors And Logic **/
  /**
   * 使当前逻辑设备闲置等待
   */
  void WaitIdle() const { vkDeviceWaitIdle(logicalDevice); }

  /** Finders And Queries **/
  /**
   * 查找当前物理设备的队列族
   */
  [[nodiscard]] QueueFamilyIndices FindQueueFamilies(
      const VkSurfaceKHR& surface) const;
  /**
   * 查询当前物理设备的交换链支持
   */
  [[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport(
      const VkSurfaceKHR& surface) const;

  /** Pickers And Creators **/
  /**
   * 创建逻辑设备的实例
   */
  void CreateLogicalDevice(const VkSurfaceKHR& surface,
                           const Validation& validation);
  /**
   * 拾取物理设备的实例
   */
  void PickPhysicalDevice(const VkInstance& instance,
                          const VkSurfaceKHR& surface);
  void DestroyLogicalDevice() const;
  VkSampleCountFlagBits GetMSAASamples() const { return msaaSamples; }

  /** Getters And Setters **/
  /**
   * 获取逻辑设备的引用
   */
  [[nodiscard]] const VkDevice& GetLogical() const { return logicalDevice; }

  /**
   * 获取物理设备的引用
   */
  [[nodiscard]] const VkPhysicalDevice& GetPhysical() const {
    return physicalDevice;
  }

  /**
   * 获取即时设备的队列
   */
  [[nodiscard]] const VkQueue& GetPresentQueue() const { return presentQueue; }

  /**
   * 获取图形设备的队列
   */
  [[nodiscard]] const VkQueue& GetGraphicsQueue() const {
    return graphicsQueue;
  }
};
