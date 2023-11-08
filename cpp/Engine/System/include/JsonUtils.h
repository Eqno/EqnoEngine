#pragma once

#include <string>

#define JSON_CONFIG(type, key) JsonUtils::Read##type##FromFile(GetRoot() + GetFile(), key)

namespace JsonUtils {
	std::string ReadStringFromFile(const std::string& filePath,
		const std::string& key);

	float ReadFloatFromFile(const std::string& filePath,
		const std::string& key);

	int ReadIntFromFile(const std::string& filePath, const std::string& key);

	void WriteStringToFile(const std::string& filePath,
		const std::string& key,
		const std::string& value);

	void AppendStringToFile(const std::string& filePath,
		const std::string& key,
		const std::string& value);

	void ClearDocumentCache();
}
