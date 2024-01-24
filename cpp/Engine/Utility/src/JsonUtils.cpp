#include "../include/JsonUtils.h"

#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Model/include/BaseModel.h>
#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Scene/include/SceneObject.h>
#include <Engine/Utility/include/FileUtils.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <ranges>
#include <unordered_map>

using namespace rapidjson;
std::unordered_map<std::string, Document*> docCache;

Document* GetJsonDocFromFile(const std::string& filePath) {
  Document* doc;
  if (const auto docIter = docCache.find(filePath); docIter != docCache.end()) {
    doc = docIter->second;
  } else {
    doc = new Document;
    doc->SetObject();
    doc->Parse(FileUtils::ReadFileAsString(filePath).c_str());
    docCache[filePath] = doc;
  }
  return doc;
}

std::string JsonUtils::ReadStringFromFile(const std::string& filePath,
                                          const std::string& key) {
  if (Document* doc = GetJsonDocFromFile(filePath);
      doc->HasMember(key.c_str())) {
    return (*doc)[key.c_str()].GetString();
  }
  return "Unset";
}

float JsonUtils::ReadFloatFromFile(const std::string& filePath,
                                   const std::string& key) {
  if (Document* doc = GetJsonDocFromFile(filePath);
      doc->HasMember(key.c_str())) {
    return (*doc)[key.c_str()].GetFloat();
  }
  return 0;
}

int JsonUtils::ReadIntFromFile(const std::string& filePath,
                               const std::string& key) {
  if (Document* doc = GetJsonDocFromFile(filePath);
      doc->HasMember(key.c_str())) {
    return (*doc)[key.c_str()].GetInt();
  }
  return 0;
}

std::vector<std::string> JsonUtils::ReadStringsFromFile(
    const std::string& filePath, const std::string& key) {
  if (Document* doc = GetJsonDocFromFile(filePath);
      doc->HasMember(key.c_str())) {
    std::vector<std::string> ret;
    const auto& values = (*doc)[key.c_str()];
    for (unsigned int i = 0; i < values.Size(); ++i) {
      ret.emplace_back(values[i].GetString());
    }
    return ret;
  }
  return {};
}

#define DEFINE_ParseGLMVec_NUM(num)                            \
  glm::vec##num ParseGLMVec##num(const std::string& content) { \
    glm::vec##num ret;                                         \
    int count = 0;                                             \
    std::string part;                                          \
    for (const char c : content) {                             \
      if (c == ',') {                                          \
        ret[count++] = stof(part);                             \
        part.clear();                                          \
      } else {                                                 \
        part.push_back(c);                                     \
      }                                                        \
    }                                                          \
    ret[count++] = stof(part);                                 \
    return ret;                                                \
  }
DEFINE_ParseGLMVec_NUM(2);
DEFINE_ParseGLMVec_NUM(3);
DEFINE_ParseGLMVec_NUM(4);
#undef DEFINE_ParseGLMVec_NUM

void TravelSceneObjectTree(GraphicsInterface* graphics, SceneObject*& parent,
                           const Value& val, const std::string& root,
                           BaseObject* owner) {
  if (val.HasMember("Type") && val.HasMember("Path")) {
    SceneObject* object = nullptr;
    if (strcmp(val["Type"].GetString(), "BaseModel") == 0) {
      object = BaseObject::CreateImmediately<BaseModel>(
          graphics, parent,
          val.HasMember("Name") ? val["Name"].GetString() : "Unset", root,
          val["Path"].GetString(), owner);
      if (val.HasMember("Camera")) {
        dynamic_cast<BaseModel*>(object)->SetCamera(val["Camera"].GetString());
      }
    } else if (strcmp(val["Type"].GetString(), "BaseCamera") == 0) {
      object = BaseObject::CreateImmediately<BaseCamera>(
          graphics, parent,
          val.HasMember("Name") ? val["Name"].GetString() : "Unset", root,
          val["Path"].GetString(), owner);
      if (val.HasMember("Transform")) {
        dynamic_cast<BaseCamera*>(object)->InitRotation(
            ParseGLMVec3(val["Transform"]["Rotation"].GetString()));
      }
    } else {
      throw std::runtime_error("unknown scene object type!");
    }
    if (val.HasMember("Transform")) {
      const auto& trans = val["Transform"];
      if (trans.HasMember("Scale")) {
        object->SetRelativeScale(ParseGLMVec3(trans["Scale"].GetString()));
      }
      if (trans.HasMember("Rotation")) {
        object->SetRelativeRotation(
            ParseGLMVec3(trans["Rotation"].GetString()));
      }
      if (trans.HasMember("Position")) {
        object->SetRelativePosition(
            ParseGLMVec3(trans["Position"].GetString()));
      }
    }
    if (val.HasMember("Sons")) {
      const auto& values = val["Sons"];
      for (unsigned int i = 0; i < values.Size(); ++i) {
        TravelSceneObjectTree(graphics, object, values[i], root, owner);
      }
    }
  } else {
    throw std::runtime_error("scene object is incomplete!");
  }
}

void JsonUtils::ParseSceneObjectTree(GraphicsInterface* graphics,
                                     SceneObject*& parent,
                                     const std::string& path,
                                     const std::string& root,
                                     BaseObject* owner) {
  if (Document* doc = GetJsonDocFromFile(path);
      doc->HasMember("SceneObjects")) {
    const auto& values = (*doc)["SceneObjects"];
    for (unsigned int i = 0; i < values.Size(); ++i) {
      TravelSceneObjectTree(graphics, parent, values[i], root, owner);
    }
  }
}

std::pair<std::string, Strings> JsonUtils::ParseMeshDataInfos(
    const std::string& filePath, const std::string& meshName) {
  std::pair<std::string, Strings> infos("Unset", Strings());
  if (Document* doc = GetJsonDocFromFile(filePath);
      doc->HasMember("Material")) {
    infos.first = (*doc)["Material"].GetString();
  }
  if (Document* doc = GetJsonDocFromFile(filePath); doc->HasMember("Meshes")) {
    if (const Value& val = (*doc)["Meshes"]; val.HasMember(meshName.c_str())) {
      const Value& info = (*doc)["Meshes"][meshName.c_str()];
      if (info.HasMember("Material")) {
        infos.first = info["Material"].GetString();
      }
      if (info.HasMember("Textures")) {
        const Value& texs = info["Textures"];
        for (unsigned int i = 0; i < texs.Size(); ++i) {
          infos.second.emplace_back(texs[i].GetString());
        }
      }
    }
  }
  return infos;
}

void JsonUtils::ParseMaterialParams(const std::string& filePath,
                                    MaterialData& matData) {
  if (Document* doc = GetJsonDocFromFile(filePath); doc->HasMember("Params")) {
    const auto& params = (*doc)["Params"];
    if (params.HasMember("Color")) {
      matData.color = ParseGLMVec4(params["Color"].GetString());
    }
    if (params.HasMember("Roughness")) {
      matData.roughness = params["Roughness"].GetFloat();
    }
    if (params.HasMember("Metallic")) {
      matData.metallic = params["Metallic"].GetFloat();
    }
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
  for (const std::string& val : values) {
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
    (*doc)[key.c_str()].SetString(
        value.c_str(), static_cast<unsigned int>(value.size()), allocator);
  } else {
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
  }

  StringBuffer strBuf;
  Writer writer(strBuf);

  doc->Accept(writer);
  FileUtils::WriteFileAsString(filePath, strBuf.GetString());
}

void JsonUtils::ClearDocumentCache() {
  for (const auto val : docCache | std::views::values) {
    delete val;
  }
  docCache.clear();
}
