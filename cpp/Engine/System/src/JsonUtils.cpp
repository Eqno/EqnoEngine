#include "../include/JsonUtils.h"

#include <rapidjson/document.h>

#include "Engine/System/include/Application.h" 

void JsonUtils::WriteToFile(std::string& filePath,
	std::string& key,
	std::string& value) {
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
	FileUtils::WriteFileAsString(Application::GetGameRoot() + "Index", data);
}
