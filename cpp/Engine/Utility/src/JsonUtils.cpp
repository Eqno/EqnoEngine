#include "../include/JsonUtils.h"

#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Light/include/SpotLight.h>
#include <Engine/Light/include/SunLight.h>
#include <Engine/Model/include/BaseModel.h>
#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Scene/include/SceneObject.h>
#include <Engine/Utility/include/FileUtils.h>

#include <ranges>
#include <unordered_map>

using namespace rapidjson;
std::unordered_map<std::string, std::shared_ptr<Document>> docCache;

std::shared_ptr<Document> JsonUtils::GetJsonDocFromFile(
    const std::string& filePath) {
  std::shared_ptr<Document> doc;
  if (const auto docIter = docCache.find(filePath); docIter != docCache.end()) {
    doc = docIter->second;
  } else {
    doc = std::make_shared<Document>();
    doc->SetObject();
    doc->Parse(FileUtils::ReadFileAsString(filePath + FILESUFFIX).c_str());
    docCache[filePath] = doc;
  }
  return doc;
}

std::string JsonUtils::ReadStringFromFile(const std::string& filePath,
                                          const std::string& key) {
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember(key.c_str())) {
    return (*doc)[key.c_str()].GetString();
  }
  return "Unset";
}

bool JsonUtils::ReadBoolFromFile(const std::string& filePath,
                                 const std::string& key) {
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember(key.c_str())) {
    return (*doc)[key.c_str()].GetBool();
  }
  return false;
}

float JsonUtils::ReadFloatFromFile(const std::string& filePath,
                                   const std::string& key) {
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember(key.c_str())) {
    return (*doc)[key.c_str()].GetFloat();
  }
  return 0;
}

int JsonUtils::ReadIntFromFile(const std::string& filePath,
                               const std::string& key) {
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember(key.c_str())) {
    return (*doc)[key.c_str()].GetInt();
  }
  return 0;
}

std::vector<std::string> JsonUtils::ReadStringsFromFile(
    const std::string& filePath, const std::string& key) {
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember(key.c_str())) {
    std::vector<std::string> ret;
    const auto& values = (*doc)[key.c_str()];
    for (SizeType i = 0; i < values.Size(); ++i) {
      ret.emplace_back(values[i].GetString());
    }
    return ret;
  }
  return {};
}

void JsonUtils::UseDefaultTransform(std ::weak_ptr<SceneObject> objectPtr,
                                    const std::string& filePath) {
  if (auto object = objectPtr.lock()) {
    if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
        doc->HasMember("DefaultTransform")) {
      const auto& trans = (*doc)["DefaultTransform"];
      if (trans.HasMember("Scale")) {
        object->SetRelativeScale(ParseGLMVec3(trans["Scale"].GetString()));
      }
      if (trans.HasMember("Rotation")) {
        object->SetRelativeRotation(
            glm::radians(ParseGLMVec3(trans["Rotation"].GetString())));
      }
      if (trans.HasMember("Position")) {
        object->SetRelativePosition(
            ParseGLMVec3(trans["Position"].GetString()));
      }
    }
  }
}

void TravelSceneObjectTree(std::weak_ptr<GraphicsInterface> graphics,
                           std::shared_ptr<SceneObject> parent,
                           const Value& val, const std::string& root,
                           std::weak_ptr<BaseObject> owner) {
  if (val.HasMember("Type") && val.HasMember("Path")) {
    std::shared_ptr<SceneObject> object;
    if (strcmp(val["Type"].GetString(), "BaseModel") == 0) {
      object = BaseObject::CreateImmediately<BaseModel>(
          graphics, parent,
          val.HasMember("Name") ? val["Name"].GetString() : "Unset", root,
          val["Path"].GetString(), owner);
      if (val.HasMember("Camera")) {
        std::static_pointer_cast<BaseModel>(object)->SetCamera(
            val["Camera"].GetString());
      }
      if (val.HasMember("LightChannel")) {
        std::static_pointer_cast<BaseModel>(object)->SetLightChannel(
            val["LightChannel"].GetString());
      }
    } else if (strcmp(val["Type"].GetString(), "BaseCamera") == 0) {
      object = BaseObject::CreateImmediately<BaseCamera>(
          graphics, parent,
          val.HasMember("Name") ? val["Name"].GetString() : "Unset", root,
          val["Path"].GetString(), owner);
      if (val.HasMember("Transform")) {
        std::static_pointer_cast<BaseCamera>(object)->InitRotation(glm::radians(
            ParseGLMVec3(val["Transform"]["Rotation"].GetString())));
      }
      if (val.HasMember("Params")) {
        auto& params = val["Params"];
        if (params.HasMember("FOVy")) {
          std::static_pointer_cast<BaseCamera>(object)->SetFOVy(
              params["FOVy"].GetFloat());
        }
        if (params.HasMember("Aspect")) {
          std::static_pointer_cast<BaseCamera>(object)->SetAspect(
              BaseCamera::ParseAspect(params["Aspect"].GetString()));
        }
        if (params.HasMember("Near")) {
          std::static_pointer_cast<BaseCamera>(object)->SetNear(
              params["Near"].GetFloat());
        }
        if (params.HasMember("Far")) {
          std::static_pointer_cast<BaseCamera>(object)->SetFar(
              params["Far"].GetFloat());
        }
        if (params.HasMember("MaxFov")) {
          std::static_pointer_cast<BaseCamera>(object)->SetMaxFov(
              params["MaxFov"].GetFloat());
        }
        if (params.HasMember("MinFov")) {
          std::static_pointer_cast<BaseCamera>(object)->SetMinFov(
              params["MinFov"].GetFloat());
        }
        if (params.HasMember("SensitivityX")) {
          std::static_pointer_cast<BaseCamera>(object)->SetSensitivityX(
              params["SensitivityX"].GetFloat());
        }
        if (params.HasMember("SensitivityY")) {
          std::static_pointer_cast<BaseCamera>(object)->SetSensitivityY(
              params["SensitivityY"].GetFloat());
        }
        if (params.HasMember("SensitivityZ")) {
          std::static_pointer_cast<BaseCamera>(object)->SetSensitivityZ(
              params["SensitivityZ"].GetFloat());
        }
        if (params.HasMember("MoveSpeed")) {
          std::static_pointer_cast<BaseCamera>(object)->SetMoveSpeed(
              params["MoveSpeed"].GetFloat());
        }
        if (params.HasMember("speedIncreasingRate")) {
          std::static_pointer_cast<BaseCamera>(object)->SetSpeedIncreasingRate(
              params["SpeedIncreasingRate"].GetFloat());
        }
        if (params.HasMember("MaxMoveSpeed")) {
          std::static_pointer_cast<BaseCamera>(object)->SetMaxMoveSpeed(
              params["MaxMoveSpeed"].GetFloat());
        }
        if (params.HasMember("MinMoveSpeed")) {
          std::static_pointer_cast<BaseCamera>(object)->SetMinMoveSpeed(
              params["MinMoveSpeed"].GetFloat());
        }
      }
    } else if (strcmp(val["Type"].GetString(), "SpotLight") == 0) {
      object = BaseObject::CreateImmediately<SpotLight>(
          graphics, parent,
          val.HasMember("Name") ? val["Name"].GetString() : "Unset", root,
          val["Path"].GetString(), owner);
      if (val.HasMember("Params")) {
        auto& params = val["Params"];
        if (params.HasMember("Intensity")) {
          std::static_pointer_cast<SpotLight>(object)->SetIntensity(
              params["Intensity"].GetFloat());
        }
        if (params.HasMember("Color")) {
          std::static_pointer_cast<SpotLight>(object)->SetColor(
              ParseGLMVec4(params["Color"].GetString()));
        }
        if (params.HasMember("FOVy")) {
          std::static_pointer_cast<SpotLight>(object)->SetFovy(
              params["FOVy"].GetFloat());
        }
        if (params.HasMember("Aspect")) {
          std::static_pointer_cast<SpotLight>(object)->SetAspect(
              BaseCamera::ParseAspect(params["Aspect"].GetString()));
        }
        if (params.HasMember("Near")) {
          std::static_pointer_cast<SpotLight>(object)->SetNear(
              params["Near"].GetFloat());
        }
        if (params.HasMember("Far")) {
          std::static_pointer_cast<SpotLight>(object)->SetFar(
              params["Far"].GetFloat());
        }
      }
    } else if (strcmp(val["Type"].GetString(), "SunLight") == 0) {
      object = BaseObject::CreateImmediately<SunLight>(
          parent, val.HasMember("Name") ? val["Name"].GetString() : "Unset",
          root, val["Path"].GetString(), owner);
      if (val.HasMember("Params")) {
        auto& params = val["Params"];
        if (params.HasMember("Intensity")) {
          std::static_pointer_cast<SunLight>(object)->SetIntensity(
              params["Intensity"].GetFloat());
        }
        if (params.HasMember("Color")) {
          std::static_pointer_cast<SunLight>(object)->SetColor(
              ParseGLMVec4(params["Color"].GetString()));
        }
        if (params.HasMember("Left")) {
          std::static_pointer_cast<SunLight>(object)->SetLeft(
              params["Left"].GetFloat());
        }
        if (params.HasMember("Right")) {
          std::static_pointer_cast<SunLight>(object)->SetRight(
              params["Right"].GetFloat());
        }
        if (params.HasMember("Bottom")) {
          std::static_pointer_cast<SunLight>(object)->SetBottom(
              params["Bottom"].GetFloat());
        }
        if (params.HasMember("Top")) {
          std::static_pointer_cast<SunLight>(object)->SetTop(
              params["Top"].GetFloat());
        }
        if (params.HasMember("Near")) {
          std::static_pointer_cast<SunLight>(object)->SetNear(
              params["Near"].GetFloat());
        }
        if (params.HasMember("Far")) {
          std::static_pointer_cast<SunLight>(object)->SetFar(
              params["Far"].GetFloat());
        }
      }
    } else {
      PRINT_AND_THROW_ERROR("unknown scene object type!");
    }
    if (object && val.HasMember("Transform")) {
      const auto& trans = val["Transform"];
      if (trans.HasMember("Scale")) {
        object->SetRelativeScale(ParseGLMVec3(trans["Scale"].GetString()));
      }
      if (trans.HasMember("Rotation")) {
        object->SetRelativeRotation(
            glm::radians(ParseGLMVec3(trans["Rotation"].GetString())));
      }
      if (trans.HasMember("Position")) {
        object->SetRelativePosition(
            ParseGLMVec3(trans["Position"].GetString()));
      }
    }
    if (val.HasMember("Sons")) {
      const auto& values = val["Sons"];
      for (SizeType i = 0; i < values.Size(); ++i) {
        TravelSceneObjectTree(graphics, object, values[i], root, owner);
      }
    }
  } else {
    PRINT_AND_THROW_ERROR("scene object is incomplete!");
  }
}

void JsonUtils::ParseSceneObjectTree(std::weak_ptr<GraphicsInterface> graphics,
                                     std::shared_ptr<SceneObject> parent,
                                     const std::string& root,
                                     const std::string& file,
                                     std::weak_ptr<BaseObject> owner) {
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(root + file);
      doc->HasMember("SceneObjects")) {
    const auto& values = (*doc)["SceneObjects"];
    for (SizeType i = 0; i < values.Size(); ++i) {
      TravelSceneObjectTree(graphics, parent, values[i], root, owner);
    }
  }
}

void JsonUtils::ParseSceneLightChannels(const std::string& root,
                                        const std::string& file,
                                        std::weak_ptr<BaseScene> owner) {
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(root + file);
      doc->HasMember("LightChannels")) {
    const auto& channels = (*doc)["LightChannels"];
    for (auto iter = channels.MemberBegin(); iter != channels.MemberEnd();
         ++iter) {
      std::shared_ptr<LightChannel> object =
          BaseObject::CreateImmediately<LightChannel>(iter->name.GetString(),
                                                      false, root, file, owner);
      for (SizeType i = 0; i < iter->value.Size(); ++i) {
        if (auto ownerPtr = owner.lock()) {
          std::weak_ptr<BaseLight> light =
              ownerPtr->GetLightByName(iter->value[i].GetString());
          object->AddLightToChannel(light);
        }
      }
    }
  }
  if (auto ownerPtr = owner.lock()) {
    if (!ownerPtr->GetLightChannelByName("All").lock()) {
      std::shared_ptr<LightChannel> object =
          BaseObject::CreateImmediately<LightChannel>("All", false, root, file,
                                                      owner);
      for (auto light : ownerPtr->GetLights() | std::views::values) {
        object->AddLightToChannel(light);
      }
    }
  }
}

MaterialInfo JsonUtils::ParseMeshDataInfos(const std::string& filePath,
                                           const std::string& meshName) {
  MaterialInfo infos("Assets/Materials/ShaderErrorMaterial", {});
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember("Material")) {
    infos.first = (*doc)["Material"].GetString();
  }
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember("Meshes")) {
    if (const Value& val = (*doc)["Meshes"]; val.HasMember(meshName.c_str())) {
      const Value& info = (*doc)["Meshes"][meshName.c_str()];
      if (info.HasMember("Material")) {
        infos.first = info["Material"].GetString();
      }
      if (info.HasMember("Textures")) {
        const Value& texs = info["Textures"];
        for (auto iter = texs.MemberBegin(); iter != texs.MemberEnd(); iter++) {
          infos.second.push_back(
              {iter->name.GetString(), iter->value.GetString()});
        }
      }
    }
  }
  return infos;
}

void JsonUtils::ParseMaterialShaders(const std::string& filePath,
                                     std::vector<std::string>& shaders) {
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember("Shaders")) {
    const auto& shaderPaths = (*doc)["Shaders"];
    for (SizeType i = 0; i < shaderPaths.Size(); ++i) {
      shaders.emplace_back(shaderPaths[i].GetString());
    }
  }
  shaders.push_back("Assets/Shaders/DefaultLit/ShaderError");
}

void JsonUtils::ParseMaterialParams(const std::string& filePath,
                                    glm::vec4& color, float& roughness,
                                    float& metallic) {
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember("Params")) {
    const auto& values = (*doc)["Params"];
    if (values.HasMember("Color")) {
      color = ParseGLMVec4(values["Color"].GetString());
    }
    if (values.HasMember("Roughness")) {
      roughness = values["Roughness"].GetFloat();
    }
    if (values.HasMember("Metallic")) {
      metallic = values["Metallic"].GetFloat();
    }
  }
}

void JsonUtils::WriteStringToFile(const std::string& filePath,
                                  const std::string& key,
                                  const std::string& value) {
  std::shared_ptr<Document> doc = std::make_shared<Document>();
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
  std::shared_ptr<Document> doc = std::make_shared<Document>();
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

void JsonUtils::WriteBoolToFile(const std::string& filePath,
                                const std::string& key, bool value) {
  std::shared_ptr<Document> doc = std::make_shared<Document>();
  docCache[filePath] = doc;

  doc->SetObject();
  auto& allocator = doc->GetAllocator();

  Value strKey(kStringType);
  strKey.SetString(key.c_str(), static_cast<unsigned int>(key.size()),
                   allocator);

  Value strValue(kStringType);
  strValue.SetBool(value);

  if (strKey.IsNull() || strValue.IsNull()) {
    return;
  }
  doc->AddMember(strKey, strValue, allocator);

  StringBuffer strBuf;
  Writer writer(strBuf);

  doc->Accept(writer);
  FileUtils::WriteFileAsString(filePath + FILESUFFIX, strBuf.GetString());
}

void JsonUtils::WriteDocumentToFile(const std::string& filePath,
                                    std::weak_ptr<Document> docPtr,
                                    bool withSuffix) {
  if (auto doc = docPtr.lock()) {
    StringBuffer strBuf;
    Writer writer(strBuf);

    doc->Accept(writer);
    FileUtils::WriteFileAsString(filePath + (withSuffix ? FILESUFFIX : ""),
                                 strBuf.GetString());
  }
}

void JsonUtils::AppendStringToFile(const std::string& filePath,
                                   const std::string& key,
                                   const std::string& value) {
  std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
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
  FileUtils::WriteFileAsString(filePath + FILESUFFIX, strBuf.GetString());
}

void JsonUtils::ModifyBoolOfFile(const std::string& filePath,
                                 const std::string& key, bool value) {
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember(key.c_str())) {
    (*doc)[key.c_str()].SetBool(value);

    StringBuffer strBuf;
    Writer writer(strBuf);

    doc->Accept(writer);
    FileUtils::WriteFileAsString(filePath + FILESUFFIX, strBuf.GetString());
  }
}

void JsonUtils::ModifyIntOfFile(const std::string& filePath,
                                const std::string& key, int value) {
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember(key.c_str())) {
    (*doc)[key.c_str()].SetInt(value);

    StringBuffer strBuf;
    Writer writer(strBuf);

    doc->Accept(writer);
    FileUtils::WriteFileAsString(filePath + FILESUFFIX, strBuf.GetString());
  }
}

void JsonUtils::ClearDocumentCache() { docCache.clear(); }

std::unordered_map<TextureType, std::string> JsonUtils::GetCombineTextures(
    const std::string& filePath) {
  std::unordered_map<TextureType, std::string> res;
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember("CombineTextures")) {
    const auto& combineInfo = (*doc)["CombineTextures"];
    if (combineInfo.HasMember("BaseColor")) {
      res[TextureType::BaseColor] = combineInfo["BaseColor"].GetString();
    }
    if (combineInfo.HasMember("Roughness")) {
      res[TextureType::Roughness] = combineInfo["Roughness"].GetString();
    }
    if (combineInfo.HasMember("Metallic")) {
      res[TextureType::Metallic] = combineInfo["Metallic"].GetString();
    }
    if (combineInfo.HasMember("Normal")) {
      res[TextureType::Normal] = combineInfo["Normal"].GetString();
    }
    if (combineInfo.HasMember("AO")) {
      res[TextureType::AO] = combineInfo["AO"].GetString();
    }
  }
  return res;
}

std::unordered_map<std::string, std::string> JsonUtils::GetMaterialMap(
    const std::string& filePath) {
  std::unordered_map<std::string, std::string> res;
  if (std::shared_ptr<Document> doc = GetJsonDocFromFile(filePath);
      doc->HasMember("MaterialMap")) {
    const auto& mapInfo = (*doc)["MaterialMap"];
    for (auto iter = mapInfo.MemberBegin(); iter != mapInfo.MemberEnd();
         ++iter) {
      res[iter->name.GetString()] = iter->value.GetString();
    }
  }
  return res;
}