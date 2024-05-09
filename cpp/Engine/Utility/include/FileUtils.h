#pragma once

#include <sstream>
#include <string>

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
}  // namespace FileUtils
