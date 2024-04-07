#include "../include/swapchain.h"

#include <algorithm>
#include <array>
#include <ranges>
#include <stdexcept>

#include "../include/depth.h"
#include "../include/device.h"
#include "../include/draw.h"
#include "../include/render.h"
#include "../include/texture.h"
#include "../include/window.h"

bool SwapChain::GetEnableZPrePass() const {
  return static_cast<Render*>(owner)->GetEnableZPrePass();
}
bool SwapChain::GetEnableShadowMap() const {
  return static_cast<Render*>(owner)->GetEnableShadowMap();
}

uint32_t SwapChain::GetShadowMapWidth() const {
  return static_cast<Render*>(owner)->GetShadowMapWidth();
}
uint32_t SwapChain::GetShadowMapHeight() const {
  return static_cast<Render*>(owner)->GetShadowMapHeight();
}

const VkRenderPass& SwapChain::GetColorRenderPass() const {
  return static_cast<Render*>(owner)->GetColorRenderPass();
}
const VkRenderPass& SwapChain::GetZPrePassRenderPass() const {
  return static_cast<Render*>(owner)->GetZPrePassRenderPass();
}
const VkRenderPass& SwapChain::GetShadowMapRenderPass() const {
  return static_cast<Render*>(owner)->GetShadowMapRenderPass();
}

VkSurfaceFormatKHR SwapChain::ChooseSurfaceFormat(
    const SurfaceFormats& availableFormats) const {
  for (const auto& format : availableFormats) {
    if (format.format == surfaceFormat &&
        format.colorSpace == surfaceColorSpace) {
      return format;
    }
  }
  return availableFormats[0];
}

VkPresentModeKHR SwapChain::ChoosePresentMode(
    const PresentModes& availablePresentModes) {
  for (const auto& mode : availablePresentModes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return mode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::ChooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities, const Window& window) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }

  auto [width, height] = window.GetFrameBufferSize();
  return {std::clamp(static_cast<uint32_t>(width),
                     capabilities.minImageExtent.width,
                     capabilities.maxImageExtent.width),
          std::clamp(static_cast<uint32_t>(height),
                     capabilities.minImageExtent.height,
                     capabilities.maxImageExtent.height)};
}

void SwapChain::CreateSwapChain(const Device& device, const Window& window) {
  const auto [capabilities, formats, presentModes] =
      device.QuerySwapChainSupport(window.GetSurface());
  const auto [format, colorSpace] = ChooseSurfaceFormat(formats);
  const auto presentMode = ChoosePresentMode(presentModes);
  const auto newExtent = ChooseSwapExtent(capabilities, window);

  auto imageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 &&
      imageCount > capabilities.maxImageCount) {
    imageCount = capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = window.GetSurface(),
      .minImageCount = imageCount,
      .imageFormat = format,
      .imageColorSpace = colorSpace,
      .imageExtent = newExtent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform = capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = presentMode,
      .clipped = VK_TRUE,
  };

  const auto [graphicsFamily, presentFamily] =
      device.FindQueueFamilies(window.GetSurface());

  const uint32_t queueFamilyIndices[] = {
      graphicsFamily.has_value() ? graphicsFamily.value() : 0,
      presentFamily.has_value() ? presentFamily.value() : 0};
  if (graphicsFamily != presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  if (vkCreateSwapchainKHR(device.GetLogical(), &createInfo, nullptr, &chain) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  vkGetSwapchainImagesKHR(device.GetLogical(), chain, &imageCount, nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device.GetLogical(), chain, &imageCount,
                          swapChainImages.data());

  imageFormat = format;
  extent = newExtent;
}

void SwapChain::CreateImageViews(const VkDevice& device) {
  swapChainImageViews.resize(swapChainImages.size());
  for (size_t i = 0; i < swapChainImages.size(); i++) {
    swapChainImageViews[i] = Texture::CreateImageView(
        device, swapChainImages[i], 1, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

void SwapChain::DestroyColorResource(const Device& device) const {
  if (device.GetMSAASamples() == VK_SAMPLE_COUNT_1_BIT) {
    return;
  }
  vkDestroyImageView(device.GetLogical(), colorImageView, nullptr);
  vkDestroyImage(device.GetLogical(), colorImage, nullptr);
  vkFreeMemory(device.GetLogical(), colorImageMemory, nullptr);
}

void SwapChain::CleanupRenderTarget(const VkDevice& device) const {
  for (const auto& frameBuffer : colorFrameBuffers) {
    vkDestroyFramebuffer(device, frameBuffer, nullptr);
  }
  if (GetEnableZPrePass()) {
    vkDestroyFramebuffer(device, zPrePassFrameBuffer, nullptr);
  }
  if (GetEnableShadowMap()) {
    for (const auto& frameBuffer : shadowMapFrameBuffers) {
      vkDestroyFramebuffer(device, frameBuffer, nullptr);
    }
  }
  for (const auto& imageView : swapChainImageViews) {
    vkDestroyImageView(device, imageView, nullptr);
  }
  vkDestroySwapchainKHR(device, chain, nullptr);
}

void SwapChain::CreateColorFrameBuffers(const Device& device) {
  colorFrameBuffers.resize(swapChainImageViews.size());
  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    std::vector attachments = {swapChainImageViews[i],
                               zPrePassDepth.GetDepthImageView()};
    if (device.GetMSAASamples() != VK_SAMPLE_COUNT_1_BIT) {
      attachments = {colorImageView, zPrePassDepth.GetDepthImageView(),
                     swapChainImageViews[i]};
    }
    VkFramebufferCreateInfo frameBufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = GetColorRenderPass(),
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .width = extent.width,
        .height = extent.height,
        .layers = 1,
    };
    if (vkCreateFramebuffer(device.GetLogical(), &frameBufferInfo, nullptr,
                            &colorFrameBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create frame buffer!");
    }
  }
}
void SwapChain::CreateZPrePassFrameBuffer(const VkDevice& device) {
  VkFramebufferCreateInfo frameBufferInfo{
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = GetZPrePassRenderPass(),
      .attachmentCount = 1,
      .pAttachments = &zPrePassDepth.GetDepthImageView(),
      .width = extent.width,
      .height = extent.height,
      .layers = 1,
  };
  if (vkCreateFramebuffer(device, &frameBufferInfo, nullptr,
                          &zPrePassFrameBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create frame buffer!");
  }
}
void SwapChain::CreateShadowMapFrameBuffers(const VkDevice& device) {
  shadowMapFrameBuffers.resize(shadowMapDepths.size());
  for (size_t i = 0; i < shadowMapDepths.size(); i++) {
    VkFramebufferCreateInfo frameBufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = GetShadowMapRenderPass(),
        .attachmentCount = 1,
        .pAttachments = &shadowMapDepths[i].GetDepthImageView(),
        .width = GetShadowMapWidth(),
        .height = GetShadowMapHeight(),
        .layers = 1,
    };
    if (vkCreateFramebuffer(device, &frameBufferInfo, nullptr,
                            &shadowMapFrameBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create frame buffer!");
    }
  }
}

void SwapChain::RecreateSwapChain(
    const Device& device, const Window& window,
    std::unordered_map<std::string, Draw*>& draws) {
  window.OnRecreateSwapChain();
  device.WaitIdle();

  DestroyColorResource(device);
  DestroyDepthResource(device.GetLogical());
  CleanupRenderTarget(device.GetLogical());

  CreateRenderTarget(device, window);
  CreateColorResource(device);
  CreateDepthResources(device);
  TransitionDepthImageLayout(device);

  if (GetEnableShadowMap()) {
    for (Draw* draw : draws | std::views::values) {
      for (const auto& mesh : draw->GetMeshes()) {
        mesh->UpdateColorDescriptorSets(device.GetLogical(),
                                        *static_cast<Render*>(owner));
      }
    }
  }
  CreateFrameBuffers(device);
}

void SwapChain::CreateRenderTarget(const Device& device, const Window& window) {
  CreateSwapChain(device, window);
  CreateImageViews(device.GetLogical());
}

void SwapChain::CreateRenderTarget(const std::string& format,
                                   const std::string& space,
                                   const Device& device, const Window& window) {
  surfaceFormat = VulkanUtils::ParseImageFormat(format);
  surfaceColorSpace = VulkanUtils::ParseColorSpace(space);
  CreateSwapChain(device, window);
  CreateImageViews(device.GetLogical());
}

void SwapChain::CreateColorResource(const Device& device) {
  if (device.GetMSAASamples() == VK_SAMPLE_COUNT_1_BIT) {
    return;
  }
  auto [image, memory] = Texture::CreateImage(
      device, extent.width, extent.height, 1, device.GetMSAASamples(),
      imageFormat, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  colorImage = image;
  colorImageMemory = memory;
  colorImageView =
      Texture::CreateImageView(device.GetLogical(), colorImage, 1, imageFormat,
                               VK_IMAGE_ASPECT_COLOR_BIT);
}

void SwapChain::CreateDepthResources(const Device& device) {
  zPrePassDepth.CreateDepthResources(
      device, extent.width, extent.height, device.GetMSAASamples(),
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

  if (GetEnableShadowMap()) {
    for (Depth& depth : shadowMapDepths) {
      depth.CreateDepthResources(device, GetShadowMapWidth(),
                                 GetShadowMapHeight(), VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_SAMPLED_BIT);
    }
  }
}

void SwapChain::TransitionDepthImageLayout(const Device& device) {
  zPrePassDepth.TransitionDepthImageLayout(
      device, *static_cast<Render*>(owner), VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  if (GetEnableShadowMap()) {
    for (Depth& depth : shadowMapDepths) {
      depth.TransitionDepthImageLayout(
          device, *static_cast<Render*>(owner), VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
  }
}

void SwapChain::CreateFrameBuffers(const Device& device) {
  CreateColorFrameBuffers(device);
  if (GetEnableZPrePass()) {
    CreateZPrePassFrameBuffer(device.GetLogical());
  }
  if (GetEnableShadowMap()) {
    CreateShadowMapFrameBuffers(device.GetLogical());
  }
}