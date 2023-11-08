#include "../include/JsonUtils.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "../include/FileUtils.h"

#include <ranges>
#include <unordered_map>

#include "Engine/Model/include/BaseModel.h"
#include "Engine/Scene/include/SceneObject.h"

using namespace rapidjson;
std::unordered_map<std::string, Document*> docCache;

Document* GetJsonDocFromFile(const std::string& filePath) {
	Document* doc;
	if (const auto docIter = docCache.find(filePath); docIter != docCache.
		end()) {
		doc = docIter->second;
	}
	else {
		doc = new Document;
		doc->SetObject();
		doc->Parse(FileUtils::ReadFileAsString(filePath).c_str());
		docCache[filePath] = doc;
	}
	return doc;
}

std::string JsonUtils::ReadStringFromFile(const std::string& filePath,
	const std::string& key) {
	if (Document* doc = GetJsonDocFromFile(filePath); doc->HasMember(
		key.c_str())) {
		return (*doc)[key.c_str()].GetString();
	}
	return "Unset";
}

float JsonUtils::ReadFloatFromFile(const std::string& filePath,
	const std::string& key) {
	if (Document* doc = GetJsonDocFromFile(filePath); doc->HasMember(
		key.c_str())) {
		return (*doc)[key.c_str()].GetFloat();
	}
	return 0;
}

int JsonUtils::ReadIntFromFile(const std::string& filePath,
	const std::string& key) {
	if (Document* doc = GetJsonDocFromFile(filePath); doc->HasMember(
		key.c_str())) {
		return (*doc)[key.c_str()].GetInt();
	}
	return 0;
}

std::vector<std::string> JsonUtils::ReadStringsFromFile(
	const std::string& filePath,
	const std::string& key) {
	if (Document* doc = GetJsonDocFromFile(filePath); doc->HasMember(
		key.c_str())) {
		std::vector<std::string> ret;
		const auto& values = (*doc)[key.c_str()];
		for (unsigned int i = 0; i < values.Size(); ++i) {
			ret.emplace_back(values[i].GetString());
		}
		return ret;
	}
	return {};
}

void TravelSceneObjectTree(const Value& val,
	const std::string& root,
	SceneObject*& parent) {
	if (val.HasMember("Type") && val.HasMember("Path")) {
		if (strcmp(val["Type"].GetString(), "BaseModel") == 0) {
			SceneObject* object = new BaseModel(root, val["Path"].GetString(),
				parent);
			if (val.HasMember("Sons")) {
				const auto& values = val["Sons"];
				for (unsigned int i = 0; i < values.Size(); ++i) {
					TravelSceneObjectTree(values[i], root, object);
				}
			}
		}
	}
	else {
		throw std::runtime_error("unknown scene object type!");
	}
}

void JsonUtils::ParseSceneObjectTree(const std::string& filePath,
	const std::string& root,
	SceneObject*& parent) {
	if (Document* doc = GetJsonDocFromFile(filePath); doc->HasMember(
		"SceneObjects")) {
		const Value& val = (*doc)["SceneObjects"];
		TravelSceneObjectTree(val, root, parent);
	}
}

void JsonUtils::WriteStringToFile(const std::string& filePath,
	const std::string& key,
	const std::string& value) {
	auto* doc = new Document;
	docCache[filePath] = doc;

	doc->SetObject();
	auto& allocator = doc->GetAllocator();

	Value strKey(kStringType);
	strKey.SetString(key.c_str(), static_cast<unsigned int>(key.size()),
		allocator);

	Value strValue(kStringType);
	strValue.SetString(value.c_str(), static_cast<unsigned int>(value.size()),
		allocator);

	if (strKey.IsNull() || strValue.IsNull()) {
		return;
	}
	doc->AddMember(strKey, strValue, allocator);

	StringBuffer strBuf;
	Writer writer(strBuf);

	doc->Accept(writer);
	FileUtils::WriteFileAsString(filePath, strBuf.GetString());
}

void JsonUtils::WriteStringsToFile(const std::string& filePath,
	const std::string& key,
	const std::vector<std::string>& values) {
	auto* doc = new Document;
	docCache[filePath] = doc;

	doc->SetObject();
	auto& allocator = doc->GetAllocator();

	Value strKey(kStringType);
	strKey.SetString(key.c_str(), static_cast<unsigned int>(key.size()),
		allocator);

	Value strValue(kArrayType);
	Value strValIter(kStringType);
	for (const std::string& val: values) {
		strValIter.SetString(val.c_str(), static_cast<unsigned int>(val.size()),
			allocator);
		if (strValIter.IsNull()) return;
		strValue.PushBack(strValIter, allocator);
	}

	if (strKey.IsNull() || strValue.IsNull()) {
		return;
	}
	doc->AddMember(strKey, strValue, allocator);

	StringBuffer strBuf;
	Writer writer(strBuf);

	doc->Accept(writer);
	FileUtils::WriteFileAsString(filePath, strBuf.GetString());
}

void JsonUtils::AppendStringToFile(const std::string& filePath,
	const std::string& key,
	const std::string& value) {
	Document* doc = GetJsonDocFromFile(filePath);
	auto& allocator = doc->GetAllocator();

	if (doc->HasMember(key.c_str())) {
		(*doc)[key.c_str()].SetString(value.c_str(),
			static_cast<unsigned int>(value.size()), allocator);
	}
	else {
		Value strKey(kStringType);
		strKey.SetString(key.c_str(), static_cast<unsigned int>(key.size()),
			allocator);

		Value strValue(kStringType);
		strValue.SetString(value.c_str(),
			static_cast<unsigned int>(value.size()), allocator);

		if (strKey.IsNull() || strValue.IsNull()) {
			return;
		}
		doc->AddMember(strKey, strValue, allocator);
	}

	StringBuffer strBuf;
	Writer writer(strBuf);

	doc->Accept(writer);
	FileUtils::WriteFileAsString(filePath, strBuf.GetString());
}

void JsonUtils::ClearDocumentCache() {
	for (const auto val: docCache | std::views::values) {
		delete val;
	}
	docCache.clear();
}
