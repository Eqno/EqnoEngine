#include "../include/BaseModel.h"

#define STB_IMAGE_IMPLEMENTATION
#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Model/include/BaseMaterial.h>
#include <Engine/Utility/include/FileUtils.h>
#include <Engine/Utility/include/JsonUtils.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/types.h>
#include <stb_image.h>

#include <assimp/Importer.hpp>

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
  if (meshData->textures.empty()) {
    throw std::runtime_error("please set textures for model or mesh!");
  }
}

void ParseFbxData(const aiMatrix4x4& transform, const aiMesh* mesh,
                  MeshData* meshData) {
  for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
    const aiVector3D pos = transform * mesh->mVertices[i];
    const aiColor4D color = mesh->mColors[0]
                                ? mesh->mColors[0][i]
                                : aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
    const aiVector3D normal =
        transform *
        (mesh->mNormals ? mesh->mNormals[i] : aiVector3D(0.0f, 0.0f, 1.0f));
    const aiVector3D tangent = mesh->mTangents ? transform * mesh->mTangents[i]
                                               : aiVector3D(1.0f, 0.0f, 0.0f);
    const aiVector3D texCoord = mesh->mTextureCoords[0]
                                    ? mesh->mTextureCoords[0][i]
                                    : aiVector3D(0.0f, 0.0f, 0.0f);
    meshData->vertices.emplace_back(pos, color, normal, tangent, texCoord);
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
                   std::vector<MeshData*>& modelMeshes) {
  const aiMatrix4x4 nodeTransform = transform * node->mTransformation;

  for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
    const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

    const auto [fst, snd] = JsonUtils::ParseMeshDataInfos(rootPath + modelPath,
                                                          mesh->mName.C_Str());

    // Parse Name
    auto meshData =
        new MeshData{.state = {.alive = true}, .name = mesh->mName.C_Str()};

    // Parse Data
    ParseFbxData(nodeTransform, mesh, meshData);

    // Parse Material
    auto* mat = modelScene->GetMaterialByPath(fst);
    meshData->material = mat->GetData();

    if (snd.empty()) {
      // Parse Textures
      ParseFbxTextures(scene->mMaterials[mesh->mMaterialIndex], meshData);
    } else {
      // Parse Textures
      for (const std::string& texPath : snd) {
        LoadPNGTexture((rootPath + texPath).c_str(), meshData->textures);
      }
    }
    modelMeshes.emplace_back(meshData);
  }

  for (unsigned int i = 0; i < node->mNumChildren; ++i) {
    ParseFbxDatas(nodeTransform, node->mChildren[i], scene, rootPath, modelPath,
                  modelScene, modelMeshes);
  }
}

void BaseModel::LoadFbxDatas(const std::string& fbxPath,
                             const unsigned int parserFlags) {
  Assimp::Importer importer;

  const aiScene* scene = importer.ReadFile(GetRoot() + fbxPath, parserFlags);
  if (scene == nullptr) {
  }

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    throw std::runtime_error(std::string("failed to load fbx model: ") +
                             importer.GetErrorString());
  }

  const aiMatrix4x4 identity;
  ParseFbxDatas(identity, scene->mRootNode, scene, GetRoot(), GetFile(),
                dynamic_cast<BaseScene*>(_owner), meshes);
}

void BaseModel::OnCreate() {
  SceneObject::OnCreate();

  if (JSON_CONFIG(String, "Type") == "FBX") {
    LoadFbxDatas(JSON_CONFIG(String, "File"),
                 aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                     aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
  } else if (JSON_CONFIG(String, "Type") == "OBJ") {
    // LoadObjDatas(JSON_CONFIG(String, "File"), 0);
  }
  dynamic_cast<BaseScene*>(_owner)->GetGraphics()->ParseMeshDatas(meshes);
}

void BaseModel::OnUpdate() {
  SceneObject::OnUpdate();

  for (MeshData* mesh : meshes) {
    mesh->uniform.modelMatrix = GetAbsoluteTransform();
    mesh->uniform.viewMatrix =
        camera ? camera->GetViewMatrix() : glm::mat4x4(0);
    mesh->uniform.projMatrix =
        camera ? camera->GetProjMatrix() : glm::mat4x4(0);
  }
}

void BaseModel::OnDestroy() {
  SceneObject::OnDestroy();

  SceneObject::OnDestroy();
  for (const MeshData* mesh : meshes) {
    delete mesh;
  }
}

void BaseModel::SetCamera(const std::string& cameraName) {
  camera = BaseCamera::CameraMap[cameraName];
}
