#pragma once

#include <iostream>
#include <string>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "FileUtils.h"

namespace JsonUtils {
	void WriteToFile(std::string& filePath,
		std::string& key,
		std::string& value);
}
