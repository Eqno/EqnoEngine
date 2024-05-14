#pragma once
#pragma once

#include <Engine/Utility/include/TypeUtils.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <glm/glm.hpp>
#include <string>
#include <vector>

class SceneObject;
class BaseObject;
class BaseScene;
class GraphicsInterface;

struct MaterialData;

#define FILESUFFIX ".json"

#define JSON_CONFIG(type, key) \
  JsonUtils::Read##type##FromFile(GetRoot() + GetFile(), key)

#define DEFINE_ParseGLMVec_NUM(num)                                   \
  inline glm::vec##num ParseGLMVec##num(const std::string& content) { \
    glm::vec##num ret;                                                \
    int count = 0;                                                    \
    std::string part;                                                 \
    for (const char c : content) {                                    \
      if (c == ',') {                                                 \
        ret[count++] = stof(part);                                    \
        part.clear();                                                 \
      } else {                                                        \
        part.push_back(c);                                            \
      }                                                               \
    }                                                                 \
    ret[count++] = stof(part);                                        \
    return ret;                                                       \
  }
DEFINE_ParseGLMVec_NUM(2);
DEFINE_ParseGLMVec_NUM(3);
DEFINE_ParseGLMVec_NUM(4);
#undef DEFINE_ParseGLMVec_NUM

namespace JsonUtils {
std::shared_ptr<rapidjson::Document> GetJsonDocFromFile(
    const std::string& filePath);
std::string ReadStringFromFile(const std::string& filePath,
                               const std::string& key);
std::vector<std::string> ReadStringsFromFile(const std::string& filePath,
                                             const std::string& key);

bool ReadBoolFromFile(const std::string& filePath, const std::string& key);
float ReadFloatFromFile(const std::string& filePath, const std::string& key);
int ReadIntFromFile(const std::string& filePath, const std::string& key);

void UseDefaultTransform(std ::weak_ptr<SceneObject> objectPtr,
                         const std::string& filePath);
void ParseSceneObjectTree(std::weak_ptr<GraphicsInterface> graphics,
                          std::shared_ptr<SceneObject> parent,
                          const std::string& root, const std::string& file,
                          std::weak_ptr<BaseObject> owner);
void ParseSceneLightChannels(const std::string& root, const std::string& file,
                             std::weak_ptr<BaseScene> owner);

MaterialInfo ParseMeshDataInfos(const std::string& filePath,
                                const std::string& meshName);

void ParseMaterialShaders(const std::string& filePath,
                          std::vector<std::string>& shaders);
void ParseMaterialParams(const std::string& filePath, glm::vec4& color,
                         float& roughness, float& metallic);

void WriteStringToFile(const std::string& filePath, const std::string& key,
                       const std::string& value);
void WriteStringsToFile(const std::string& filePath, const std::string& key,
                        const std::vector<std::string>& values);
void WriteBoolToFile(const std::string& filePath, const std::string& key,
                     bool value);
void WriteDocumentToFile(const std::string& filePath,
                         std::weak_ptr<rapidjson::Document> docPtr,
                         bool withSuffix = true);
void AppendStringToFile(const std::string& filePath, const std::string& key,
                        const std::string& value);
void ModifyBoolOfFile(const std::string& filePath, const std::string& key,
                      bool value);
void ModifyIntOfFile(const std::string& filePath, const std::string& key,
                     int value);
void ClearDocumentCache();
std::unordered_map<TextureType, std::string> GetCombineTextures(
    const std::string& filePath);
std::unordered_map<std::string, std::string> GetMaterialMap(
    const std::string& filePath);
}  // namespace JsonUtils
