#include "../include/swapchain.h"

#include <algorithm>
#include <stdexcept>
#include <array>

#include "../include/depth.h"
#include "../include/device.h"
#include "../include/texture.h"
#include "../include/window.h"

VkSurfaceFormatKHR SwapChain::ChooseSurfaceFormat(
	const SurfaceFormats& availableFormats) {
	for (const auto& format: availableFormats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace ==
		    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR SwapChain::ChoosePresentMode(
	const PresentModes& availablePresentModes) {
	for (const auto& mode: availablePresentModes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return mode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::ChooseSwapExtent(
	const VkSurfaceCapabilitiesKHR& capabilities,
	const Window& window) {
	if (capabilities.currentExtent.width != std::numeric_limits<
		    uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	auto [width, height] = window.GetFrameBufferSize();
	return {
		std::clamp(static_cast<uint32_t>(width),
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width),
		std::clamp(static_cast<uint32_t>(height),
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height)
	};
}

void SwapChain::Create(const Device& device, const Window& window) {
	const auto [capabilities, formats, presentModes] = device.
		QuerySwapChainSupport(window.GetSurface());
	const auto [format, colorSpace] = ChooseSurfaceFormat(formats);
	const auto presentMode = ChoosePresentMode(presentModes);
	const auto newExtent = ChooseSwapExtent(capabilities, window);

	auto imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.
	    maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo {
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
	const auto [graphicsFamily, presentFamily] = device.FindQueueFamilies(
		window.GetSurface());

	const uint32_t queueFamilyIndices[] = {
		graphicsFamily.has_value() ? graphicsFamily.value() : 0,
		presentFamily.has_value() ? presentFamily.value() : 0
	};
	if (graphicsFamily != presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	if (vkCreateSwapchainKHR(device.GetLogical(), &createInfo, nullptr, &chain)
	    != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}
	vkGetSwapchainImagesKHR(device.GetLogical(), chain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device.GetLogical(),
		chain,
		&imageCount,
		images.data());

	imageFormat = format;
	extent = newExtent;
}

void SwapChain::CreateImageViews(const VkDevice& device) {
	imageViews.resize(images.size());
	for (size_t i = 0; i < images.size(); i++) {
		imageViews[i] = Texture::CreateImageView(device,
			images[i],
			imageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void SwapChain::CleanupSwapChain(const VkDevice& device,
	const Depth& depth) const {
	depth.Destroy(device);
	for (const auto& frameBuffer: frameBuffers) {
		vkDestroyFramebuffer(device, frameBuffer, nullptr);
	}
	for (const auto& imageView: imageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(device, chain, nullptr);
}

void SwapChain::CreateFrameBuffers(const VkDevice& device,
	const Depth& depth,
	const VkRenderPass& renderPass) {
	frameBuffers.resize(imageViews.size());
	for (size_t i = 0; i < imageViews.size(); i++) {
		std::array attachments = {imageViews[i], depth.GetDepthImageView()};

		VkFramebufferCreateInfo frameBufferInfo {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.width = extent.width,
			.height = extent.height,
			.layers = 1,
		};
		if (vkCreateFramebuffer(device,
			    &frameBufferInfo,
			    nullptr,
			    &frameBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create frame buffer!");
		}
	}
}

void SwapChain::RecreateSwapChain(const Device& device,
	Depth& depth,
	const Window& window,
	const VkRenderPass& renderPass) {
	window.OnRecreateSwapChain();
	device.WaitIdle();
	CleanupSwapChain(device.GetLogical(), depth);

	Create(device, window);
	CreateImageViews(device.GetLogical());
	depth.CreateDepthResources(device, extent);
	CreateFrameBuffers(device.GetLogical(), depth, renderPass);
}
