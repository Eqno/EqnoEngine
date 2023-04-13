#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "shader.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

Shader::Shader(const Definitions& definitions) {
	for (const auto& [name, value]: definitions) {
		options.AddMacroDefinition(name, value);
	}
}

auto Shader::GetSPVPathFromGLSLPath(std::string glslPath) -> std::string {
	return glslPath.replace(glslPath.find('.'), 1, "_").append(".spv");
}

auto Shader::GetShaderTypeByGLSLSuffix(
	const std::string& glslPath
) -> std::string { return glslPath.substr(glslPath.find('.') + 1); }

auto Shader::CompileShaderFromGLSLToSPV(
	const std::string& glslPath
) const -> void {
	if (std::ifstream glslFile("shaders/glsl/" + glslPath); glslFile.
		is_open()) {
		std::stringstream buffer;
		buffer << glslFile.rdbuf();
		auto glslCode(buffer.str());

		auto module = compiler.CompileGlslToSpv(
			glslCode.c_str(),
			glslCode.size(),
			types.find(GetShaderTypeByGLSLSuffix(glslPath))->second.kind,
			glslPath.c_str(),
			options
		);

		if (module.GetCompilationStatus() !=
			shaderc_compilation_status_success) {
			std::cerr << module.GetErrorMessage();
		}
		std::vector spvCode(module.cbegin(), module.cend());

		std::ofstream spvFile(
			"shaders/spv/" + GetSPVPathFromGLSLPath(glslPath),
			std::ios::binary
		);
		spvFile.write(
			reinterpret_cast<const char*>(spvCode.data()),
			static_cast<std::streamsize>(sizeof(uint32_t) / sizeof(char) *
				spvCode.size())
		);

		spvFile.close();
		glslFile.close();
	} else { throw std::runtime_error("failed to open file!"); }
}

auto Shader::ReadSPVFileAsBinary(
	const std::string& spvPath
) -> std::vector<uint32_t> {
	if (std::ifstream file(
		"shaders/spv/" + spvPath,
		std::ios::ate | std::ios::binary
	); file.is_open()) {
		const size_t          fileSize = file.tellg();
		std::vector<uint32_t> buffer(fileSize);

		file.seekg(0);
		file.read(
			reinterpret_cast<char*>(buffer.data()),
			static_cast<std::streamsize>(fileSize)
		);
		file.close();

		return buffer;
	}
	throw std::runtime_error("failed to open file!");
}

auto Shader::ReadGLSLFileAsBinary(
	const std::string& glslPath
) const -> std::vector<uint32_t> {
	CompileShaderFromGLSLToSPV(glslPath);
	return ReadSPVFileAsBinary(GetSPVPathFromGLSLPath(glslPath));
}

auto Shader::CreateShaderModule(
	const std::vector<uint32_t>& code,
	const VkDevice&              device
) -> VkShaderModule {
	const VkShaderModuleCreateInfo createInfo {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size(),
		.pCode = code.data(),
	};

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}

auto Shader::AutoCreateShaderStages(
	const VkDevice& device
) const -> std::vector<VkPipelineShaderStageCreateInfo> {
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	shaderModules = {};
	for (const auto& fileInfo: std::filesystem::directory_iterator(
		     "shaders/glsl/"
	     )) {
		const auto  glslPath             = fileInfo.path().filename().string();
		const auto& [_, stage, entrance] = types.find(
			GetShaderTypeByGLSLSuffix(glslPath)
		)->second;
		shaderModules.emplace_back(
			CreateShaderModule(ReadGLSLFileAsBinary(glslPath), device)
		);
		shaderStages.push_back(
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = stage,
				.module = shaderModules.back(),
				.pName = entrance.c_str(),
			}
		);
	}
	return shaderStages;
}

auto Shader::DestroyShaderModules(const VkDevice& device) const -> void {
	for (const auto& shaderModule: shaderModules) {
		vkDestroyShaderModule(device, shaderModule, nullptr);
	}
}
