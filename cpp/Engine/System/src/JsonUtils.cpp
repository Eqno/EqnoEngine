#include "../include/JsonUtils.h"

#include <ranges>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "Engine/System/include/Application.h"
#include "Engine/System/include/FileUtils.h"

using namespace rapidjson;
std::unordered_map<std::string, Document*> docCache;

std::string JsonUtils::ReadStringFromFile(const std::string& filePath,
	const std::string& key) {
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

	if (doc->HasMember(key.c_str())) {
		return (*doc)[key.c_str()].GetString();
	}
	return "Unset";
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

void JsonUtils::AppendStringToFile(const std::string& filePath,
	const std::string& key,
	const std::string& value) {
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
