#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <shaderc/shaderc.hpp>

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

	static auto GetSPVPathFromGLSLPath(std::string glslPath) -> std::string;
	static auto GetShaderTypeByGLSLSuffix(
		const std::string& glslPath
	) -> std::basic_string<char>;

	auto CompileShaderFromGLSLToSPV(const std::string& glslPath) const -> void;

	[[nodiscard]] static auto ReadSPVFileAsBinary(
		const std::string& spvPath
	) -> std::vector<uint32_t>;

	[[nodiscard]] auto ReadGLSLFileAsBinary(
		const std::string& glslPath
	) const -> std::vector<uint32_t>;

	static auto CreateShaderModule(
		const std::vector<uint32_t>& code,
		const VkDevice&              device
	) -> VkShaderModule;
	auto DestroyShaderModules(const VkDevice& device) const -> void;

	[[nodiscard]] auto AutoCreateShaderStages(
		const VkDevice& device
	) const -> std::vector<VkPipelineShaderStageCreateInfo>;
};
