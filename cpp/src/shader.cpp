#include "shader.h"

#include <filesystem>
#include <fstream>
#include <iostream>

Shader::Shader(const Definitions& definitions) {
	for (const auto& [name, value]: definitions) {
		options.AddMacroDefinition(name, value);
	}
}

auto Shader::GetTypeByName(
	const std::string& glslPath
) const -> const ShaderTypeInfo& {
	return types.find(glslPath.substr(glslPath.find('.') + 1))->second;
}

auto Shader::CompileFromGLSLToSPV(const std::string& glslPath) const -> void {
	if (std::ifstream glslFile("shaders/glsl/" + glslPath); glslFile.
		is_open()) {
		std::stringstream buffer;
		buffer << glslFile.rdbuf();
		auto glslCode(buffer.str());

		auto module = compiler.CompileGlslToSpv(
			glslCode.c_str(),
			glslCode.size(),
			GetTypeByName(glslPath).kind,
			glslPath.c_str(),
			options
		);

		if (module.GetCompilationStatus() !=
			shaderc_compilation_status_success) {
			std::cerr << module.GetErrorMessage();
		}
		std::vector spvCode(module.cbegin(), module.cend());

		std::ofstream spvFile("shaders/spv/" + glslPath, std::ios::binary);
		spvFile.write(
			reinterpret_cast<const char*>(spvCode.data()),
			static_cast<std::streamsize>(sizeof(uint32_t) / sizeof(char) *
				spvCode.size())
		);

		spvFile.close();
		glslFile.close();
	} else { throw std::runtime_error("failed to open file!"); }
}

auto Shader::ReadSPVFileAsBinary(const std::string& spvPath) -> UIntegers {
	if (std::ifstream file(
		"shaders/spv/" + spvPath,
		std::ios::ate | std::ios::binary
	); file.is_open()) {
		const size_t fileSize = file.tellg();
		UIntegers    buffer(fileSize);
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
) const -> UIntegers {
	CompileFromGLSLToSPV(glslPath);
	return ReadSPVFileAsBinary(glslPath);
}

auto Shader::CreateModule(
	const UIntegers& code,
	const VkDevice&  device
) -> VkShaderModule {
	const VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size(),
		.pCode = code.data(),
	};
	if (VkShaderModule shaderModule; vkCreateShaderModule(
		device,
		&createInfo,
		nullptr,
		&shaderModule
	) == VK_SUCCESS) { return shaderModule; }
	throw std::runtime_error("failed to create shader module!");
}

auto Shader::AutoCreateStages(const VkDevice& device) const -> ShaderStages {
	ShaderStages shaderStages;
	for (const auto& fileInfo: std::filesystem::directory_iterator(
		     "shaders/glsl/"
	     )) {
		const auto glslPath = fileInfo.path().filename().string();
		shaderModules.emplace_back(
			CreateModule(ReadGLSLFileAsBinary(glslPath), device)
		);
		shaderStages.push_back(
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = GetTypeByName(glslPath).stage,
				.module = shaderModules.back(),
				.pName = GetTypeByName(glslPath).entrance.c_str(),
			}
		);
	}
	return shaderStages;
}

auto Shader::DestroyModules(const VkDevice& device) const -> void {
	for (const auto& shaderModule: shaderModules) {
		vkDestroyShaderModule(device, shaderModule, nullptr);
	}
	shaderModules.clear();
}
