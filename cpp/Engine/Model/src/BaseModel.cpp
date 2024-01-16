#include "../include/BaseModel.h"

#define STB_IMAGE_IMPLEMENTATION
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/types.h>
#include <stb_image.h>

#include <assimp/Importer.hpp>

#include "Engine/Model/include/BaseMaterial.h"
#include "Engine/Utility/include/FileUtils.h"

// std::unordered_map<std::string, aiMaterial*> defaultMaterialDatas;
// void BaseModel::ParseFbxMaterials(const aiScene* scene) {
// 	std::vector<std::string> materials;
// 	for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
// 		if (aiMaterial* material = scene->mMaterials[i]) {
// 			aiString name;
// 			aiGetMaterialString(material, AI_MATKEY_NAME, &name);
// 			defaultMaterialDatas[name.C_Str()] = material;
// 		}
// 	}
// }

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

    // Parse Material
    auto* mat = modelScene->GetMaterialByPath(fst);
    meshData->material = {mat->GetShader(), mat->GetParams()};

    // Parse Data
    ParseFbxData(nodeTransform, mesh, meshData);

    // Parse Textures
    for (const std::string& texPath : snd) {
      int width, height, channels;
      stbi_uc* data = stbi_load((rootPath + texPath).c_str(), &width, &height,
                                &channels, STBI_rgb_alpha);
      if (!data) {
        throw std::runtime_error("failed to load texture image!");
      }
      meshData->textures.emplace_back(width, height, channels, data);
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

  static int count = 0;
  if (count++ > 10000) {
    for (MeshData* mesh : meshes) {
      mesh->state.alive = false;
    }
  }

  glm::mat4x4 view =
      lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
             glm::vec3(0.0f, 1.0f, 0.0f));

  glm::mat4x4 proj = glm::perspective(
      glm::radians(45.0f), graphics->GetViewportAspect(), 0.1f, 1000.0f);

  for (MeshData* mesh : meshes) {
    transform.absoluteForward;
    mesh->uniform.modelMatrix = transform.getAbsoluteTransform();
    mesh->uniform.viewMatrix = view;
    mesh->uniform.projMatrix = proj;
  }
}

void BaseModel::OnDestroy() {
  SceneObject::OnDestroy();

  SceneObject::OnDestroy();
  for (const MeshData* mesh : meshes) {
    delete mesh;
  }
}
