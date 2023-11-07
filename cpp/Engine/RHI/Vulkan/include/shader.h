#pragma once

#include <unordered_map>
#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan_core.h>

#include "Engine/System/include/TypeUtils.h"

using ShaderStages = std::vector<VkPipelineShaderStageCreateInfo>;
using Definitions = std::vector<std::pair<std::string, std::string>>;

struct ShaderTypeInfo {
	shaderc_shader_kind kind;
	VkShaderStageFlagBits stage;
	std::string entrance;
};

class Shader {
	mutable std::vector<VkShaderModule> shaderModules;
	static const ShaderTypeInfo& GetTypeByName(const std::string& glslPath);

	[[nodiscard]] static UIntegers ReadSPVFileAsBinary(
		const std::string& spvPath,
		const std::string& shaderPath);

	[[nodiscard]] static UIntegers ReadGLSLFileAsBinary(
		const std::string& glslPath,
		const std::string& shaderPath);

	static void CompileFromGLSLToSPV(const std::string& glslPath,
		const std::string& shaderPath);

	static VkShaderModule CreateModule(const UIntegers& code,
		const VkDevice& device);

public:
	explicit Shader() = default;
	explicit Shader(const Definitions& definitions);

	void DestroyModules(const VkDevice& device) const;
	[[nodiscard]] ShaderStages AutoCreateStages(const VkDevice& device,
		const std::string& shaderPath) const;
};
