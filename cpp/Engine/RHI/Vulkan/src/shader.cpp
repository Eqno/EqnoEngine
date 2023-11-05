#include "../include/shader.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "../include/config.h"

namespace ShaderRcStatic {
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	std::unordered_map<std::string, ShaderTypeInfo> shaderTypes {
		{
			"vert",
			{shaderc_glsl_vertex_shader, VK_SHADER_STAGE_VERTEX_BIT, "main"}
		},
		{
			"frag",
			{shaderc_glsl_fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT, "main"}
		},
	};
}

Shader::Shader(const Definitions& definitions) {
	for (const auto& [name, value]: definitions) {
		ShaderRcStatic::options.AddMacroDefinition(name, value);
	}
}

const ShaderTypeInfo& Shader::GetTypeByName(const std::string& glslPath) {
	return ShaderRcStatic::shaderTypes.find(
		glslPath.substr(glslPath.find('.') + 1))->second;
}

void Shader::CompileFromGLSLToSPV(const std::string& glslPath,
	const std::string& shaderPath) {
	if (std::ifstream glslFile(shaderPath + glslPath); glslFile.is_open()) {
		std::stringstream buffer;
		buffer << glslFile.rdbuf();
		auto glslCode(buffer.str());

		auto module = ShaderRcStatic::compiler.CompileGlslToSpv(
			glslCode.c_str(),
			glslCode.size(),
			GetTypeByName(glslPath).kind,
			glslPath.c_str(),
			ShaderRcStatic::options);

		if (module.GetCompilationStatus() !=
		    shaderc_compilation_status_success) {
			std::cerr << module.GetErrorMessage();
		}
		std::vector spvCode(module.cbegin(), module.cend());

		std::ofstream spvFile(Config::SHADER_PATH + "../spv/" + glslPath,
			std::ios::binary);
		spvFile.write(reinterpret_cast<const char*>(spvCode.data()),
			sizeof(uint32_t) / sizeof(char) * spvCode.size());

		spvFile.close();
		glslFile.close();
	}
	else {
		throw std::runtime_error("failed to open file!");
	}
}

UIntegers Shader::ReadSPVFileAsBinary(const std::string& spvPath,
	const std::string& shaderPath) {
	if (std::ifstream file(shaderPath + "../spv/" + spvPath,
		std::ios::ate | std::ios::binary); file.is_open()) {
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

UIntegers Shader::ReadGLSLFileAsBinary(const std::string& glslPath,
	const std::string& shaderPath) {
	CompileFromGLSLToSPV(glslPath, shaderPath);
	return ReadSPVFileAsBinary(glslPath, shaderPath);
}

VkShaderModule Shader::CreateModule(const UIntegers& code,
	const VkDevice& device) {
	const VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size(),
		.pCode = code.data(),
	};
	if (VkShaderModule shaderModule; vkCreateShaderModule(device,
		                                 &createInfo,
		                                 nullptr,
		                                 &shaderModule) == VK_SUCCESS) {
		return shaderModule;
	}
	throw std::runtime_error("failed to create shader module!");
}

ShaderStages Shader::AutoCreateStages(const VkDevice& device,
	const std::string& shaderPath) const {
	ShaderStages shaderStages;
	for (const auto& fileInfo:
	     std::filesystem::directory_iterator(shaderPath)) {
		const auto glslPath = fileInfo.path().filename().string();
		shaderModules.emplace_back(
			CreateModule(ReadGLSLFileAsBinary(glslPath, shaderPath), device));
		shaderStages.push_back({
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = GetTypeByName(glslPath).stage,
			.module = shaderModules.back(),
			.pName = GetTypeByName(glslPath).entrance.c_str(),
		});
	}
	return shaderStages;
}

void Shader::DestroyModules(const VkDevice& device) const {
	for (const auto& shaderModule: shaderModules) {
		vkDestroyShaderModule(device, shaderModule, nullptr);
	}
	shaderModules.clear();
}
