#pragma once

#include <string>
#include <vector>

class SceneObject;
class BaseScene;

#define JSON_CONFIG(type, key) JsonUtils::Read##type##FromFile(GetRoot() + GetFile(), key)

namespace JsonUtils {
	std::string ReadStringFromFile(const std::string& filePath,
		const std::string& key);

	std::vector<std::string> ReadStringsFromFile(const std::string& filePath,
		const std::string& key);

	float ReadFloatFromFile(const std::string& filePath,
		const std::string& key);

	int ReadIntFromFile(const std::string& filePath, const std::string& key);

	void ParseSceneObjectTree(const std::string& filePath,
		const std::string& root,
		SceneObject*& parent,
		BaseScene* inScene);

	std::pair<std::string, std::vector<std::string>> ParseMeshDataInfos(
		const std::string& filePath,
		const std::string& meshName);

	void WriteStringToFile(const std::string& filePath,
		const std::string& key,
		const std::string& value);

	void WriteStringsToFile(const std::string& filePath,
		const std::string& key,
		const std::vector<std::string>& values);

	void AppendStringToFile(const std::string& filePath,
		const std::string& key,
		const std::string& value);

	void ClearDocumentCache();
}
