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

std ::vector<BaseLight*> LightsEmpty;

#define LoadPNGTexture(path, textures)                                 \
  {                                                                    \
    int width, height, channels;                                       \
    stbi_uc* data =                                                    \
        stbi_load((path), &width, &height, &channels, STBI_rgb_alpha); \
    if (!data) {                                                       \
      throw std::runtime_error("failed to load texture image!");       \
    }                                                                  \
    (textures).emplace_back(width, height, channels, data);            \
  }

void ParseFbxTextures(aiMaterial* material, MeshData* meshData) {
  if (material == nullptr) {
    return;
  }
  for (int i = 0; i <= aiTextureType::AI_TEXTURE_TYPE_MAX; i++) {
    int textureCount = material->GetTextureCount(static_cast<aiTextureType>(i));
    for (int j = 0; j < textureCount; j++) {
      aiString path;
      if (material->GetTexture(static_cast<aiTextureType>(i), j, &path) ==
          AI_SUCCESS) {
        LoadPNGTexture(path.C_Str(), meshData->textures);
      }
    }
  }
}

void ParseFbxData(const aiMatrix4x4& transform, const aiMesh* mesh,
                  MeshData* meshData, float importSize) {
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
    meshData->vertices.emplace_back(pos * importSize, color, normal, tangent,
                                    texCoord);
  }
  for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
    const auto& face = mesh->mFaces[i];
    for (size_t j = 0; j < 3; ++j) {
      meshData->indices.emplace_back(face.mIndices[j]);
    }
  }
}

void ParseFbxDatas(const aiMatrix4x4& transform, const aiNode* node,
                   const aiScene* scene, const std::string& rootPath,
                   const std::string& modelPath, BaseScene* modelScene,
                   std::vector<MeshData*>& modelMeshes, float importSize) {
  const aiMatrix4x4 nodeTransform = transform * node->mTransformation;

  for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
    const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    const auto [matPath, texPaths] = JsonUtils::ParseMeshDataInfos(
        rootPath + modelPath, mesh->mName.C_Str());

    // Parse Name
    MeshData* meshData =
        new MeshData{.state = {.alive = true}, .name = mesh->mName.C_Str()};

    // Parse Data
    ParseFbxData(nodeTransform, mesh, meshData, importSize);

    // Get Material Data
    aiMaterial* matData = nullptr;
    if (scene->HasMaterials() && mesh->mMaterialIndex < scene->mNumMaterials) {
      matData = scene->mMaterials[mesh->mMaterialIndex];
    }

    if (texPaths.empty()) {
      // Parse Textures
      ParseFbxTextures(matData, meshData);
    } else {
      // Parse Textures
      for (const std::string& texPath : texPaths) {
        LoadPNGTexture((rootPath + texPath).c_str(), meshData->textures);
      }
    }

    // Parse Material
    BaseMaterial* mat = modelScene->GetMaterialByPath(matPath, matData);
    meshData->uniform.shaders = &mat->GetShaders();
    meshData->uniform.material = &mat->GetParams();

    modelMeshes.emplace_back(meshData);
  }

  for (unsigned int i = 0; i < node->mNumChildren; ++i) {
    ParseFbxDatas(nodeTransform, node->mChildren[i], scene, rootPath, modelPath,
                  modelScene, modelMeshes, importSize);
  }
}

void BaseModel::LoadFbxDatas(const unsigned int parserFlags) {
  Assimp::Importer importer;

  const aiScene* scene =
      importer.ReadFile(GetRoot() + JSON_CONFIG(String, "File"), parserFlags);

  if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      scene->mRootNode == nullptr) {
    throw std::runtime_error(std::string("failed to load fbx model: ") +
                             importer.GetErrorString());
  }

  const aiMatrix4x4 identity;
  ParseFbxDatas(identity, scene->mRootNode, scene, GetRoot(), GetFile(),
                dynamic_cast<BaseScene*>(_owner), meshes,
                JSON_CONFIG(Float, "ImportSize"));
}

void BaseModel::OnCreate() {
  SceneObject::OnCreate();

  if (JSON_CONFIG(String, "Type") == "FBX") {
    LoadFbxDatas(aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                 aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
  } else if (JSON_CONFIG(String, "Type") == "OBJ") {
    // LoadObjDatas(JSON_CONFIG(String, "File"), 0);
  }
  scene->GetGraphics()->ParseMeshDatas(meshes);
}

void BaseModel::OnStart() {
  SceneObject::OnStart();

  BaseCamera* camera = GetCamera();
  LightChannel* channel = GetLightChannel();

  for (MeshData* mesh : meshes) {
    mesh->uniform.lights = channel ? &channel->GetLights() : &LightsEmpty;

    mesh->uniform.cameraPosition =
        camera ? &camera->GetAbsolutePosition() : &Vec3Zero;
    mesh->uniform.cameraForward =
        camera ? &camera->GetAbsoluteForward() : &Vec3Zero;

    mesh->uniform.modelMatrix = &GetAbsoluteTransform();
    mesh->uniform.viewMatrix = camera ? &camera->GetViewMatrix() : &Mat4x4Zero;
    mesh->uniform.projMatrix = camera ? &camera->GetProjMatrix() : &Mat4x4Zero;
  }
}

void BaseModel::OnUpdate() { SceneObject::OnUpdate(); }

void BaseModel::OnDestroy() {
  SceneObject::OnDestroy();

  for (const MeshData* mesh : meshes) {
    delete mesh;
  }
}

void BaseModel::SetCamera(const std::string& cameraName) {
  _cameraName = cameraName;
}
void BaseModel::SetLightChannel(const std::string& lightChannelName) {
  _lightChannelName = lightChannelName;
}

BaseCamera* BaseModel::GetCamera() {
  if (_camera != nullptr) {
    return _camera;
  } else {
    return scene->GetCameraByName(_cameraName);
  }
}
LightChannel* BaseModel::GetLightChannel() {
  if (_lightChannel != nullptr) {
    return _lightChannel;
  } else {
    return scene->GetLightChannelByName(_lightChannelName);
  }
}

#undef LoadPNGTexture