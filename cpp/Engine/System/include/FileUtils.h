#pragma once

#include <string>
#include <sstream>

#include "TypeUtils.h"

namespace FileUtils {
	UIntegers ReadFileAsUIntegers(const std::string& filePath,
		int readMode = std::ios::binary);

	std::string ReadFileAsString(const std::string& filePath);

	void WriteFileAsUIntegers(const std::string& filePath,
		int writeMode = std::ios::binary,
		const UIntegers& fileContent = {});

	void WriteFileAsString(const std::string& filePath,
		const std::string& fileContent = {});

	const std::string& GetEngineFilePath(const std::string& fileName);
	const std::string& GetGameFilePath(const std::string& fileName);

	void AddGameFilePath();
}
