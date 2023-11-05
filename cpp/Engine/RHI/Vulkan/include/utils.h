#pragma once

#include <set>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

class Vertex;

using Vertexes = std::vector<Vertex>;

using Integers = std::vector<int32_t>;
using UIntegers = std::vector<uint32_t>;

using Strings = std::vector<std::string>;
using CStrings = std::vector<const char*>;

using StringSet = std::set<std::string>;

namespace VulkanUtils {
	inline VkFormat ParseImageFormat(const std::string& imageFormat) {
		if (imageFormat == "SRGB") {
			return VK_FORMAT_R8G8B8A8_SRGB;
		}
		return VK_FORMAT_R8G8B8A8_UNORM;
	}

	inline VkColorSpaceKHR ParseColorSpace(const std::string& colorSpace) {
		if (colorSpace == "SRGB") {
			return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		}
		return VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
	}
}
