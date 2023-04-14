#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan_core.h>

#include "utils.h"

using ShaderStages = std::vector<VkPipelineShaderStageCreateInfo>;
using Definitions = std::vector<std::pair<std::string, std::string>>;

struct ShaderTypeInfo {
	shaderc_shader_kind   kind;
	VkShaderStageFlagBits stage;
	std::string           entrance;
};

class Shader {
	shaderc::Compiler                   compiler;
	shaderc::CompileOptions             options;
	mutable std::vector<VkShaderModule> shaderModules;

	const std::unordered_map<std::string, ShaderTypeInfo> types {
		{
			"vert",
			{ shaderc_glsl_vertex_shader, VK_SHADER_STAGE_VERTEX_BIT, "main" }
		},
		{
			"frag",
			{
				shaderc_glsl_fragment_shader,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				"main"
			}
		},
	};
public:
	explicit Shader() = default;
	explicit Shader(const Definitions& definitions);

	auto GetTypeByName(
		const std::string& glslPath
	) const -> const ShaderTypeInfo&;

	auto CompileFromGLSLToSPV(const std::string& glslPath) const -> void;

	[[nodiscard]] static auto ReadSPVFileAsBinary(
		const std::string& spvPath
	) -> UIntegers;

	[[nodiscard]] auto ReadGLSLFileAsBinary(
		const std::string& glslPath
	) const -> UIntegers;

	static auto CreateModule(
		const UIntegers& code,
		const VkDevice&  device
	) -> VkShaderModule;

	auto DestroyModules(const VkDevice& device) const -> void;

	[[nodiscard]] auto AutoCreateStages(
		const VkDevice& device
	) const -> ShaderStages;
};
