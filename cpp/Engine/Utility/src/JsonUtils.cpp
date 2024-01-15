#include "../include/JsonUtils.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <ranges>
#include <unordered_map>

#include "../include/FileUtils.h"
#include "Engine/Model/include/BaseModel.h"
#include "Engine/Scene/include/BaseScene.h"
#include "Engine/Scene/include/SceneObject.h"

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

void TravelSceneObjectTree(BaseObject* owner, const Value& val,
                           const std::string& root, SceneObject*& parent,
                           GraphicsInterface* graphics) {
  if (val.HasMember("Type") && val.HasMember("Path")) {
    if (strcmp(val["Type"].GetString(), "BaseModel") == 0) {
      SceneObject* object = BaseObject::CreateImmediately<BaseModel>(
          graphics, parent, root, val["Path"].GetString(), owner);
      if (val.HasMember("Sons")) {
        const auto& values = val["Sons"];
        for (unsigned int i = 0; i < values.Size(); ++i) {
          TravelSceneObjectTree(owner, values[i], root, object, graphics);
        }
      }
    }
  } else {
    throw std::runtime_error("unknown scene object type!");
  }
}

void JsonUtils::ParseSceneObjectTree(BaseObject* owner,
                                     const std::string& filePath,
                                     const std::string& root,
                                     SceneObject*& parent,
                                     GraphicsInterface* graphics) {
  if (Document* doc = GetJsonDocFromFile(filePath);
      doc->HasMember("SceneObjects")) {
    const Value& val = (*doc)["SceneObjects"];
    TravelSceneObjectTree(owner, val, root, parent, graphics);
  }
}

std::pair<std::string, std::vector<std::string>> JsonUtils::ParseMeshDataInfos(
    const std::string& filePath, const std::string& meshName) {
  std::pair<std::string, std::vector<std::string>> infos;
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

std::vector<std::string> JsonUtils::ParseMaterialParams(
    const std::string& filePath) {
  std::vector<std::string> outParams;
  if (Document* doc = GetJsonDocFromFile(filePath); doc->HasMember("Params")) {
    const Value& params = (*doc)["Params"];
    for (unsigned int i = 0; i < params.Size(); ++i) {
      if (params[i].HasMember("Key") && params[i].HasMember("Type") &&
          params[i].HasMember("Value")) {
        const Value& param = params[i];
        outParams.emplace_back(param["Key"].GetString());
        outParams.emplace_back(param["Type"].GetString());
        outParams.emplace_back(param["Value"].GetString());
      }
    }
  }
  return outParams;
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
