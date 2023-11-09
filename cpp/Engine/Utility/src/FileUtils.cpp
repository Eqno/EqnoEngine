#include "../include/FileUtils.h"

#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include "Engine/System/include/Application.h"

UIntegers FileUtils::ReadFileAsUIntegers(const std::string& filePath,
	const int readMode) {
	if (std::ifstream file(filePath, readMode); file.is_open()) {
		const size_t fileSize = file.tellg();
		UIntegers buffer(fileSize);
		file.seekg(0);
		file.read(reinterpret_cast<char*>(buffer.data()),
			static_cast<std::streamsize>(fileSize));
		file.close();
		return buffer;
	}
	throw std::runtime_error("failed to open file: " + filePath);
}

std::string FileUtils::ReadFileAsString(const std::string& filePath) {
	if (std::ifstream file(filePath); file.is_open()) {
		std::stringstream buffer;
		buffer << file.rdbuf();

		file.close();
		return buffer.str();
	}
	throw std::runtime_error("failed to open file: " + filePath);
}

void FileUtils::WriteFileAsUIntegers(const std::string& filePath,
	const int writeMode,
	const UIntegers& fileContent) {
	if (std::ofstream file(filePath, writeMode); file.is_open()) {
		file.write(reinterpret_cast<const char*>(fileContent.data()),
			sizeof(uint32_t) / sizeof(char) * fileContent.size());
		file.close();
	}
	else {
		throw std::runtime_error("failed to open file: " + filePath);
	}
}

void FileUtils::WriteFileAsString(const std::string& filePath,
	const std::string& fileContent) {
	if (std::ofstream file(filePath); file.is_open()) {
		file.write(fileContent.data(), sizeof(char) * fileContent.size());
		file.close();
	}
	else {
		throw std::runtime_error("failed to open file: " + filePath);
	}
}
