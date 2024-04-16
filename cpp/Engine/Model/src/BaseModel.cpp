#include "../include/BaseModel.h"

#define STB_IMAGE_IMPLEMENTATION

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

#include <assimp/Importer.hpp>
#include <mutex>
#include <thread>

std ::vector<BaseLight*> LightsEmpty;

#define EndProcessByRenderThread \
  if (graphics == nullptr || graphics->GetRenderLoopEnd()) return;

#define LoadPNGTexture(type, _path, textures)                           \
  {                                                                     \
    int width, height, channels;                                        \
    stbi_uc* data =                                                     \
        stbi_load((_path), &width, &height, &channels, STBI_rgb_alpha); \
    if (!data) {                                                        \
      throw std::runtime_error("failed to load texture image!");        \
    }                                                                   \
    (textures).emplace_back(type, width, height, channels, data);       \
  }

#define ParseFbxTextureType(_aiTexturetype, textureType)                       \
  {                                                                            \
    for (int j = 0; j < material->GetTextureCount(_aiTexturetype); j++) {      \
      aiString path;                                                           \
      if (material->GetTexture(_aiTexturetype, j, &path) == AI_SUCCESS) {      \
        std::string fullPath = rootPath + dataPath;                            \
        fullPath = fullPath.substr(0, fullPath.rfind('/') + 1) + path.C_Str(); \
        LoadPNGTexture(textureType, fullPath.c_str(), meshData->textures);     \
      }                                                                        \
    }                                                                          \
  }

void ParseFbxTextures(aiMaterial* material, std::shared_ptr<MeshData> meshData,
                      const std::string& rootPath, const std::string& modelPath,
                      const std::string& dataPath) {
  if (material == nullptr) {
    return;
  }
  const std::unordered_map<TextureType, std::string> combineInfo(
      JsonUtils::GetCombineTextures(rootPath + modelPath));

  if (combineInfo.empty()) {
    ParseFbxTextureType(aiTextureType::aiTextureType_DIFFUSE,
                        TextureType::BaseColor);
    ParseFbxTextureType(aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS,
                        TextureType::Roughness);
    ParseFbxTextureType(aiTextureType::aiTextureType_METALNESS,
                        TextureType::Metallic);
    ParseFbxTextureType(aiTextureType::aiTextureType_NORMALS,
                        TextureType::Normal);
    ParseFbxTextureType(aiTextureType::aiTextureType_AMBIENT_OCCLUSION,
                        TextureType::AO);
  } else {
    aiString path;
    if (material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &path) ==
        AI_SUCCESS) {
      std::string picPath = path.C_Str();
      std::string texPath = rootPath + dataPath;
      std::string prefPath = texPath.substr(0, texPath.rfind('/') + 1) +
                             picPath.substr(0, picPath.rfind('_') + 1);

      for (const auto& info : combineInfo) {
        std::string fullPath = prefPath + info.second + ".png";
        LoadPNGTexture(info.first, fullPath.c_str(), meshData->textures);
      }
    }
  }
}

void ParseFbxData(GraphicsInterface* graphics, const aiMatrix4x4& transform,
                  const aiMesh* mesh, std::shared_ptr<MeshData> meshData,
                  float importSize) {
  EndProcessByRenderThread;
  for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
    EndProcessByRenderThread;

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

    EndProcessByRenderThread;
    meshData->vertices.emplace_back(
        MathUtils::AiVector3D2GlmVec3(pos * importSize),
        MathUtils::AiColor4D2GlmVec4(color),
        MathUtils::AiVector3D2GlmVec3(normal),
        MathUtils::AiVector3D2GlmVec3(tangent),
        MathUtils::AiVector3D2GlmVec3(texCoord));
  }

  EndProcessByRenderThread;
  for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
    EndProcessByRenderThread;
    const auto& face = mesh->mFaces[i];
    for (size_t j = 0; j < 3; ++j) {
      meshData->indices.emplace_back(face.mIndices[j]);
    }
  }
}

void ParseFbxDatas(BaseModel* model, BaseScene* modelScene,
                   GraphicsInterface* graphics, const std::string& modelType,
                   float importSize, const aiMatrix4x4& transform,
                   const aiNode* node, const aiScene* scene,
                   const std::string& dataPath) {
  EndProcessByRenderThread;
  const aiMatrix4x4 nodeTransform = transform * node->mTransformation;

  for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
    EndProcessByRenderThread;

    const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    const auto [matPath, texPaths] = JsonUtils::ParseMeshDataInfos(
        model->GetRoot() + model->GetFile(), mesh->mName.C_Str());

    // Parse Name
    std::shared_ptr<MeshData> meshData = std::make_shared<MeshData>(
        MeshData({.state = {.alive = true}, .name = mesh->mName.C_Str()}));

    // Parse Data
    EndProcessByRenderThread;
    ParseFbxData(graphics, nodeTransform, mesh, meshData, importSize);

    // Get Material Data
    aiMaterial* matData = nullptr;
    if (scene->HasMaterials() && mesh->mMaterialIndex < scene->mNumMaterials) {
      matData = scene->mMaterials[mesh->mMaterialIndex];
    }

    EndProcessByRenderThread;
    if (texPaths.empty()) {
      // Parse Textures
      ParseFbxTextures(matData, meshData, model->GetRoot(), model->GetFile(),
                       dataPath);
    } else {
      // Parse Textures
      for (const auto& texPath : texPaths) {
        LoadPNGTexture(TextureTypeMap[texPath.first],
                       (model->GetRoot() + texPath.second).c_str(),
                       meshData->textures);
      }
    }

    // Parse Material
    EndProcessByRenderThread;
    meshData->uniform.material =
        modelScene->GetMaterialByPath(matPath, matData);

    // Parse MeshData
    meshData->uniform.camera = model->GetCamera();
    meshData->uniform.lightChannel = model->GetLightChannel();
    meshData->uniform.modelMatrix = &model->GetAbsoluteTransform();

    model->GetMeshes().emplace_back(meshData);
    graphics->ParseMeshData(meshData);

    std::cout << "Load mesh name: " << mesh->mName.C_Str() << std::endl;
  }

  for (unsigned int i = 0; i < node->mNumChildren; ++i) {
    ParseFbxDatas(model, modelScene, graphics, modelType, importSize,
                  nodeTransform, node->mChildren[i], scene, dataPath);
  }
}

void BaseModel::LoadFbxDatas(const unsigned int parserFlags) {
  if (auto scenePtr = scene.lock()) {
    if (auto graphicsPtr = graphics.lock()) {
      Assimp::Importer importer;

      const std::string& dataPath = JSON_CONFIG(String, "File");
      const aiScene* sceneData =
          importer.ReadFile(GetRoot() + dataPath, parserFlags);

      if (sceneData == nullptr ||
          sceneData->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
          sceneData->mRootNode == nullptr) {
        throw std::runtime_error(std::string("failed to load fbx model: ") +
                                 importer.GetErrorString());
      }

      const aiMatrix4x4 identity;
      ParseFbxDatas(this, scenePtr.get(), graphicsPtr.get(),
                    JSON_CONFIG(String, "Type"),
                    JSON_CONFIG(Float, "ImportSize"), identity,
                    sceneData->mRootNode, sceneData, dataPath);

      std::cout << "All meshes num: " << meshes.size() << std::endl;
    }
  }
}

void BaseModel::OnCreate() { SceneObject::OnCreate(); }
void BaseModel::OnStart() {
  SceneObject::OnStart();
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