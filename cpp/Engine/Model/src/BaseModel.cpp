#include "../include/BaseModel.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE2_IMPLEMENTATION

#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Model/include/BaseMaterial.h>
#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Scene/include/SceneObject.h>
#include <Engine/System/include/GraphicsInterface.h>
#include <Engine/Utility/include/FileUtils.h>
#include <Engine/Utility/include/JsonUtils.h>
#include <Engine/Utility/include/MathUtils.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/types.h>
#include <stb_image.h>
#include <stb_image_resize2.h>

#include <assimp/Importer.hpp>
#include <mutex>
#include <ranges>
#include <thread>

std ::vector<BaseLight*> LightsEmpty;

void BaseModel::ClearTextureCache() {
  auto iter = TextureCache.begin();
  while (iter != TextureCache.end()) {
    iter->second.data->inUse.lock();
    stbi_image_free(iter->second.data->content);
    iter->second.data->inUse.unlock();
    iter = TextureCache.erase(iter);
  }
}

#define LoadPNGTexture(type, _path, textures, comp)                            \
  {                                                                            \
    const auto texIter = model->TextureCache.find(_path);                      \
    if (texIter != model->TextureCache.end()) {                                \
      (textures).emplace_back(type, texIter->second.width,                     \
                              texIter->second.height,                          \
                              texIter->second.channels, texIter->second.data); \
    } else {                                                                   \
      int origWidth, origHeight, channels;                                     \
      stbi_uc* origData = stbi_load((_path), &origWidth, &origHeight,          \
                                    &channels, STBI_rgb_alpha);                \
      if (origData == nullptr) {                                               \
        PRINT_ERROR("failed to load texture image!");                          \
      } else {                                                                 \
        int width = origWidth / comp, height = origHeight / comp;              \
        stbi_uc* data = (stbi_uc*)malloc(width * height * 4);                  \
        data = stbir_resize_uint8_linear(origData, origWidth, origHeight, 0,   \
                                         data, width, height, 0, STBIR_RGBA);  \
        if (data == nullptr) {                                                 \
          PRINT_AND_THROW_ERROR("failed to resize texture image!");            \
        }                                                                      \
        std::shared_ptr<TextureDataContent> dataPtr =                          \
            std::make_shared<TextureDataContent>(data);                        \
        (textures).emplace_back(type, width, height, channels, dataPtr);       \
        model->TextureCache[_path] = {type, width, height, channels, dataPtr}; \
        stbi_image_free(origData);                                             \
      }                                                                        \
    }                                                                          \
  }

#define ParseFbxTextureType(_aiTexturetype, textureType, textureComp)          \
  {                                                                            \
    for (int j = 0; j < material->GetTextureCount(_aiTexturetype); j++) {      \
      aiString path;                                                           \
      if (material->GetTexture(_aiTexturetype, j, &path) == AI_SUCCESS) {      \
        std::string fullPath = model->GetRoot() + dataPath;                    \
        fullPath = fullPath.substr(0, fullPath.rfind('/') + 1) + path.C_Str(); \
        LoadPNGTexture(textureType, fullPath.c_str(), meshData->textures,      \
                       textureComp);                                           \
      }                                                                        \
    }                                                                          \
  }

void ParseFbxTextures(
    std::weak_ptr<BaseModel> modelPtr, aiMaterial* material,
    std::shared_ptr<MeshData> meshData, const std::string& dataPath,
    const std::unordered_map<TextureType, std::string>& combineTextures) {
  if (material == nullptr) {
    return;
  }
  auto model = modelPtr.lock();
  if (!model) {
    return;
  }

  if (combineTextures.empty()) {
    ParseFbxTextureType(aiTextureType::aiTextureType_DIFFUSE,
                        TextureType::BaseColor,
                        model->GetTextureCompressionRatio());
    ParseFbxTextureType(aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS,
                        TextureType::Roughness,
                        model->GetTextureCompressionRatio());
    ParseFbxTextureType(aiTextureType::aiTextureType_METALNESS,
                        TextureType::Metallic,
                        model->GetTextureCompressionRatio());
    ParseFbxTextureType(aiTextureType::aiTextureType_NORMALS,
                        TextureType::Normal,
                        model->GetTextureCompressionRatio());
    ParseFbxTextureType(aiTextureType::aiTextureType_AMBIENT_OCCLUSION,
                        TextureType::AO, model->GetTextureCompressionRatio());
  } else {
    aiString path;
    if (material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &path) ==
        AI_SUCCESS) {
      std::string picPath = path.C_Str();
      std::string texPath = model->GetRoot() + dataPath;
      std::string prefPath = texPath.substr(0, texPath.rfind('/') + 1) +
                             picPath.substr(0, picPath.rfind('_') + 1);

      for (const auto& info : combineTextures) {
        std::string fullPath = prefPath + info.second + ".png";
        LoadPNGTexture(info.first, fullPath.c_str(), meshData->textures,
                       model->GetTextureCompressionRatio());
      }
    }
  }
}

void ParseFbxData(const aiMatrix4x4& transform, const aiMesh* mesh,
                  std::shared_ptr<MeshData> meshData, float importSize) {
  for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
    aiVector3D pos = transform * mesh->mVertices[i];
    aiVector3D root = transform * aiVector3D(0);

    aiColor4D color = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
    if (mesh->HasVertexColors(0)) {
      color = mesh->mColors[0][i];
    }
    aiVector3D normal = aiVector3D(0.0f, 0.0f, 0.0f);
    if (mesh->HasNormals()) {
      normal = (transform * mesh->mNormals[i] - root).Normalize();
    }
    aiVector3D tangent = aiVector3D(0.0f, 0.0f, 0.0f);
    if (mesh->HasTangentsAndBitangents()) {
      tangent = (transform * mesh->mTangents[i] - root).Normalize();
    }
    aiVector3D texCoord = aiVector3D(0.0f, 0.0f, 0.0f);
    if (mesh->HasTextureCoords(0)) {
      texCoord = mesh->mTextureCoords[0][i];
    }

    meshData->vertices.emplace_back(
        MathUtils::AiVector3D2GlmVec3(pos * importSize),
        MathUtils::AiColor4D2GlmVec4(color),
        MathUtils::AiVector3D2GlmVec3(normal),
        MathUtils::AiVector3D2GlmVec3(tangent),
        MathUtils::AiVector3D2GlmVec3(texCoord));
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
    const auto& face = mesh->mFaces[i];
    for (size_t j = 0; j < 3; ++j) {
      meshData->indices.emplace_back(face.mIndices[j]);
    }
  }
}

void ParseFbxDatas(
    std::weak_ptr<BaseModel> modelPtr, std::weak_ptr<BaseScene> modelScenePtr,
    std::weak_ptr<GraphicsInterface> graphicsPtr, const std::string& modelType,
    const aiMatrix4x4& transform, const aiNode* node, const aiScene* scene,
    const std::string& dataPath,
    const std::unordered_map<TextureType, std::string>& combineTextures,
    const std::unordered_map<std::string, std::string>& materialMap) {
  const aiMatrix4x4 nodeTransform = transform * node->mTransformation;

  for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
    const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

    // Get Material Data
    aiMaterial* matData = nullptr;
    if (scene->HasMaterials() && mesh->mMaterialIndex < scene->mNumMaterials) {
      matData = scene->mMaterials[mesh->mMaterialIndex];

      const auto matItem = materialMap.find(matData->GetName().C_Str());
      if (matItem != materialMap.end() && matItem->second == "Discard") {
        continue;
      }
    }

    // Parse Info
    auto model = modelPtr.lock();
    if (!model) {
      return;
    }
    const auto [matPath, texPaths] = JsonUtils::ParseMeshDataInfos(
        model->GetRoot() + model->GetFile(), mesh->mName.C_Str());

    // Parse Name
    std::shared_ptr<MeshData> meshData = std::make_shared<MeshData>(
        MeshData({.state = {.alive = true}, .name = mesh->mName.C_Str()}));

    // Parse Data
    ParseFbxData(nodeTransform, mesh, meshData, model->GetImportSize());

    if (texPaths.empty()) {
      // Parse Textures
      ParseFbxTextures(modelPtr, matData, meshData, dataPath, combineTextures);
    } else {
      // Parse Textures
      for (const auto& texPath : texPaths) {
        LoadPNGTexture(TextureTypeMap[texPath.first],
                       (model->GetRoot() + texPath.second).c_str(),
                       meshData->textures, model->GetTextureCompressionRatio());
      }
    }

    // Parse Material
    auto modelScene = modelScenePtr.lock();
    if (!modelScene) {
      return;
    }
    meshData->uniform.material =
        modelScene->GetMaterialByPath(matPath, matData);

    // Parse MeshData
    meshData->uniform.camera = model->GetCamera();
    meshData->uniform.lightChannel = model->GetLightChannel();
    meshData->uniform.modelMatrix = &model->GetAbsoluteTransform();

    model->GetMeshes().emplace_back(meshData);
    auto graphics = graphicsPtr.lock();
    if (!graphics) {
      return;
    }
    graphics->ParseMeshData(meshData);
    std::cout << "Load mesh name: " << mesh->mName.C_Str() << std::endl;
  }

  for (unsigned int i = 0; i < node->mNumChildren; ++i) {
    ParseFbxDatas(modelPtr, modelScenePtr, graphicsPtr, modelType,
                  nodeTransform, node->mChildren[i], scene, dataPath,
                  combineTextures, materialMap);
  }
}

void BaseModel::LoadFbxDatas(const unsigned int parserFlags) {
  if (auto scenePtr = scene.lock()) {
    if (auto graphicsPtr = graphics.lock()) {
      Assimp::Importer importer;
      const std::string& dataPath = JSON_CONFIG(String, "ModelFile");
      const aiScene* sceneData =
          importer.ReadFile(GetRoot() + dataPath, parserFlags);

      if (sceneData == nullptr ||
          sceneData->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
          sceneData->mRootNode == nullptr) {
        throw std::runtime_error(
            std::string("failed to load model resource: ") +
            importer.GetErrorString());
      }

      const std::unordered_map<TextureType, std::string> combineTextures(
          JsonUtils::GetCombineTextures(GetRoot() + GetFile()));

      const std::unordered_map<std::string, std::string> materialMap(
          JsonUtils::GetMaterialMap(GetRoot() + GetFile()));

      const aiMatrix4x4 identity;
      ParseFbxDatas(static_pointer_cast<BaseModel>(shared_from_this()),
                    scenePtr, graphicsPtr, JSON_CONFIG(String, "ModelType"),
                    identity, sceneData->mRootNode, sceneData, dataPath,
                    combineTextures, materialMap);

      std::cout << "All meshes num: " << meshes.size() << std::endl;
    }
  }
  loaing = false;
}

void BaseModel::OnCreate() {
  SceneObject::OnCreate();
  importSize = JSON_CONFIG(Float, "ImportSize");
  textureCompressionRatio = JSON_CONFIG(Float, "TextureCompressionRatio");
}
void BaseModel::OnStart() {
  SceneObject::OnStart();
  loaing = true;
  if (auto scenePtr = scene.lock()) {
    scenePtr->AddModelToResourceWaitQueue(
        std::function<void()>(
            std::bind(&BaseModel::LoadFbxDatas, this,
                      aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                          aiProcess_FlipUVs | aiProcess_CalcTangentSpace)),
        shared_from_this());
  }
}
void BaseModel::OnUpdate() { SceneObject::OnUpdate(); }
void BaseModel::OnStop() { SceneObject::OnStop(); }

void BaseModel::OnDestroy() {
  SceneObject::OnDestroy();
  meshes.clear();
  ClearTextureCache();
}

void BaseModel::SetCamera(const std::string& cameraName) {
  _cameraName = cameraName;
}
void BaseModel::SetLightChannel(const std::string& lightChannelName) {
  _lightChannelName = lightChannelName;
}

std::weak_ptr<BaseCamera> BaseModel::GetCamera() {
  if (_camera.lock()) {
    return _camera;
  } else if (auto scenePtr = scene.lock()) {
    return scenePtr->GetCameraByName(_cameraName);
  } else {
    return std::shared_ptr<BaseCamera>(nullptr);
  }
}
std::weak_ptr<LightChannel> BaseModel::GetLightChannel() {
  if (_lightChannel.lock()) {
    return _lightChannel;
  } else if (auto scenePtr = scene.lock()) {
    return scenePtr->GetLightChannelByName(_lightChannelName);
  } else {
    return std::shared_ptr<LightChannel>(nullptr);
  }
}

std::vector<std::shared_ptr<MeshData>>& BaseModel::GetMeshes() {
  return meshes;
}

#undef ParseFbxTextureType
#undef LoadPNGTexture