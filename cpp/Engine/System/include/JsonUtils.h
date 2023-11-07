#pragma once

#include <string>

namespace JsonUtils {
	std::string ReadStringFromFile(const std::string& filePath,
		const std::string& key);

	void WriteStringToFile(const std::string& filePath,
		const std::string& key,
		const std::string& value);

	void AppendStringToFile(const std::string& filePath,
		const std::string& key,
		const std::string& value);

	void ClearDocumentCache();
}
