#include "../include/FileUtils.h"

#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

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
	throw std::runtime_error("failed to open file!");
}

std::string FileUtils::ReadFileAsString(const std::string& filePath) {
	if (std::ifstream file(filePath); file.is_open()) {
		std::stringstream buffer;
		buffer << file.rdbuf();

		file.close();
		return buffer.str();
	}
	throw std::runtime_error("failed to open file!");
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
		throw std::runtime_error("failed to open file!");
	}
}

void FileUtils::WriteFileAsString(const std::string& filePath,
	const std::string& fileContent) {
	if (std::ofstream file(filePath); file.is_open()) {
		file.write(fileContent.data(), sizeof(char) * fileContent.size());
		file.close();
	}
	else {
		throw std::runtime_error("failed to open file!");
	}
}

const std::string& FileUtils::GetGameFilePath(const std::string& fileName) {
	return Application::GetGameRoot() + "";
}

void FileUtils::AddGameFilePath() {
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	std::string str = "Configs/Graphics";
	rapidjson::Value strValue(rapidjson::kStringType);
	strValue.SetString(str.c_str(), str.size());
	if (!strValue.IsNull()) {
		doc.AddMember("GraphicsConfig", strValue, allocator);
	}

	rapidjson::StringBuffer strBuf;
	rapidjson::Writer writer(strBuf);
	doc.Accept(writer);

	std::string data = strBuf.GetString();
	std::cout << data << std::endl;
	WriteFileAsString(Application::GetGameRoot() + "Index", data);
}
