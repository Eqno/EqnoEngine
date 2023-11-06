#pragma once

#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
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

namespace FileUtils {
	inline UIntegers ReadFileAsUIntegers(const std::string& filePath,
		const int readMode = std::ios::binary) {
		if (std::ifstream file(filePath, readMode); file.is_open()) {
			const size_t fileSize = file.tellg();
			UIntegers buffer(fileSize);
			file.seekg(0);
			file.read(reinterpret_cast<char*>(buffer.data()),
				static_cast<std::streamsize>(fileSize));
			file.close();
			return buffer;
		}
		throw std::runtime_error("failed to open file!");
	}

	inline std::string ReadFileAsString(const std::string& filePath) {
		if (std::ifstream file(filePath); file.is_open()) {
			std::stringstream buffer;
			buffer << file.rdbuf();

			file.close();
			return buffer.str();
		}
		throw std::runtime_error("failed to open file!");
	}

	inline void WriteFileAsUIntegers(const std::string& filePath,
		const int writeMode = std::ios::binary,
		const UIntegers& fileContent = {}) {
		if (std::ofstream file(filePath, writeMode); file.is_open()) {
			file.write(reinterpret_cast<const char*>(fileContent.data()),
				sizeof(uint32_t) / sizeof(char) * fileContent.size());
			file.close();
		}
		else {
			throw std::runtime_error("failed to open file!");
		}
	}
}
