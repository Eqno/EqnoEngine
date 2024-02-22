#include "../include/swapchain.h"

#include <algorithm>
#include <array>
#include <stdexcept>

#include "../include/depth.h"
#include "../include/device.h"
#include "../include/render.h"
#include "../include/texture.h"
#include "../include/window.h"

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
  colorImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device.GetLogical(), chain, &imageCount,
                          colorImages.data());

  imageFormat = format;
  extent = newExtent;
}

void SwapChain::CreateColorImageViews(const VkDevice& device) {
  colorImageViews.resize(colorImages.size());
  for (size_t i = 0; i < colorImages.size(); i++) {
    colorImageViews[i] = Texture::CreateImageView(
        device, colorImages[i], imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

void SwapChain::CleanupRenderTarget(const VkDevice& device) const {
  for (const auto& frameBuffer : colorFrameBuffers) {
    vkDestroyFramebuffer(device, frameBuffer, nullptr);
  }
  vkDestroyFramebuffer(device, zPrePassFrameBuffer, nullptr);
  for (const auto& frameBuffer : shadowMapFrameBuffers) {
    vkDestroyFramebuffer(device, frameBuffer, nullptr);
  }
  for (const auto& imageView : colorImageViews) {
    vkDestroyImageView(device, imageView, nullptr);
  }
  vkDestroySwapchainKHR(device, chain, nullptr);
}

void SwapChain::CreateColorFrameBuffers(const VkDevice& device,
                                        const VkRenderPass& colorRenderPass) {
  colorFrameBuffers.resize(colorImageViews.size());
  for (size_t i = 0; i < colorImageViews.size(); i++) {
    std::array attachments{colorImageViews[i],
                           zPrePassDepth.GetDepthImageView()};

    VkFramebufferCreateInfo frameBufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = colorRenderPass,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .width = extent.width,
        .height = extent.height,
        .layers = 1,
    };
    if (vkCreateFramebuffer(device, &frameBufferInfo, nullptr,
                            &colorFrameBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create frame buffer!");
    }
  }
}
void SwapChain::CreateZPrePassFrameBuffer(
    const VkDevice& device, const VkRenderPass& zPrePassRenderPass) {
  VkFramebufferCreateInfo frameBufferInfo{
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = zPrePassRenderPass,
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
void SwapChain::CreateShadowMapFrameBuffers(
    const VkDevice& device, const VkRenderPass& shadowMapRenderPass,
    const uint32_t shadowMapWidth, const uint32_t shadowMapHeight) {
  shadowMapFrameBuffers.resize(shadowMapDepths.size());
  for (size_t i = 0; i < shadowMapDepths.size(); i++) {
    VkFramebufferCreateInfo frameBufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = shadowMapRenderPass,
        .attachmentCount = 1,
        .pAttachments = &shadowMapDepths[i].GetDepthImageView(),
        .width = shadowMapWidth,
        .height = shadowMapHeight,
        .layers = 1,
    };
    if (vkCreateFramebuffer(device, &frameBufferInfo, nullptr,
                            &shadowMapFrameBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create frame buffer!");
    }
  }
}

void SwapChain::RecreateSwapChain(const Device& device, const Window& window,
                                  const VkRenderPass& colorRenderPass,
                                  const VkRenderPass& zPrePassRenderPass,
                                  const VkRenderPass& shadowMapRenderPass) {
  window.OnRecreateSwapChain();
  device.WaitIdle();

  DestroyDepthResource(device.GetLogical());
  CleanupRenderTarget(device.GetLogical());

  CreateRenderTarget(device, window);
  CreateDepthResources(device);
  CreateFrameBuffers(device.GetLogical(), colorRenderPass, zPrePassRenderPass,
                     shadowMapRenderPass);
}

void SwapChain::CreateRenderTarget(const Device& device, const Window& window) {
  CreateSwapChain(device, window);
  CreateColorImageViews(device.GetLogical());
}

void SwapChain::CreateRenderTarget(const std::string& format,
                                   const std::string& space,
                                   const Device& device, const Window& window) {
  surfaceFormat = VulkanUtils::ParseImageFormat(format);
  surfaceColorSpace = VulkanUtils::ParseColorSpace(space);
  CreateSwapChain(device, window);
  CreateColorImageViews(device.GetLogical());
}

void SwapChain::CreateDepthResources(const Device& device) {
  zPrePassDepth.CreateDepthResources(
      device, extent.width, extent.height,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

  for (Depth& depth : shadowMapDepths) {
    depth.CreateDepthResources(device, GetShadowMapWidth(),
                               GetShadowMapHeight(),
                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                   VK_IMAGE_USAGE_SAMPLED_BIT);
  }
}

void SwapChain::TransitionDepthImageLayout(const Device& device) {
  zPrePassDepth.TransitionDepthImageLayout(
      device, *dynamic_cast<Render*>(owner), VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  for (Depth& depth : shadowMapDepths) {
    depth.TransitionDepthImageLayout(
        device, *dynamic_cast<Render*>(owner), VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  }
}