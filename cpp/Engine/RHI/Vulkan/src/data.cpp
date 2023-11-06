#include "../include/data.h"

#include <stdexcept>
#include <tiny_obj_loader.h>
#include <unordered_map>

#include "Engine/RHI/Vulkan/include/mesh.h"
#include "Engine/RHI/Vulkan/include/utils.h"

void Data::Create(const char* dataPath) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, dataPath)) {
		throw std::runtime_error(warn + err);
	}
	std::unordered_map<Vertex, uint32_t, Vertex::HashFunction> uniqueVertices
		{};

	for (const auto& [name, mesh, lines, points]: shapes) {
		for (const auto& [vertex_index, normal_index, texcoord_index]: mesh.
		     indices) {
			glm::vec3 pos = {
				attrib.vertices[3 * vertex_index + 0],
				attrib.vertices[3 * vertex_index + 1],
				attrib.vertices[3 * vertex_index + 2],
			};
			glm::vec3 normal = {
				attrib.normals[3 * normal_index + 0],
				attrib.normals[3 * normal_index + 1],
				attrib.normals[3 * normal_index + 2]
			};
			glm::vec3 color = {1.0f, 1.0f, 1.0f};
			glm::vec2 texCoord = {
				attrib.texcoords[2 * texcoord_index + 0],
				1.0f - attrib.texcoords[2 * texcoord_index + 1],
			};
			Vertex vertex(pos, color, normal, texCoord);
			if (!uniqueVertices.contains(vertex)) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			indices.push_back(uniqueVertices[vertex]);
		}
	}
}

#include <assimp//Importer.hpp>
#include <assimp/scene.h>
#include <assimp/matrix4x4.h>

// //Materials
// static void s_ProcessMaterials(const struct aiScene* pScene,
// 	StringVector& aMaterials) {
// 	for (unsigned int i = 0; i < pScene->mNumMaterials; ++i) {
// 		aiMaterial* pMaterial = pScene->mMaterials[i];
// 		if (pMaterial) {
// 			aiString name;
// 			aiGetMaterialString(pMaterial, AI_MATKEY_NAME, &name);
// 			aMaterials.push_back(name.C_Str());
// 		}
// 	}
// }
//
// //FMeshData
// static bool s_ProcessMeshData(void* pTransform,
// 	struct aiMesh* pMesh,
// 	const struct aiScene* pScene,
// 	FMeshData& meshData) {
// 	if (pMesh->mNumVertices <= 0) return false;
//
// 	meshData.nameMesh = pMesh->mName.C_Str();
// 	aiMatrix4x4 meshTransformation = *reinterpret_cast<aiMatrix4x4*>(
// 		pTransform);
// 	for (unsigned int i = 0; i < pMesh->mNumVertices; ++i) {
// 		aiVector3D position = meshTransformation * pMesh->mVertices[i];
// 		aiColor4D color = pMesh->mColors[0]
// 		                  ? pMesh->mColors[0][i]
// 		                  : aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
// 		aiVector3D normal = meshTransformation * (pMesh->mNormals
// 		                                          ? pMesh->mNormals[i]
// 		                                          : aiVector3D(0.0f, 0.0f,
// 			                                          1.0f));
// 		aiVector3D tangent = pMesh->mTangents
// 		                     ? meshTransformation * pMesh->mTangents[i]
// 		                     : aiVector3D(1.0f, 0.0f, 0.0f);
// 		aiVector3D texCoords = pMesh->mTextureCoords[0]
// 		                       ? pMesh->mTextureCoords[0][i]
// 		                       : aiVector3D(0.0f, 0.0f, 0.0f);
//
// 		FMeshVertex vertex(position.x, position.y, position.z, color.r, color.g,
// 			color.b, color.a, normal.x, normal.y, normal.z, tangent.x,
// 			tangent.y, tangent.z, texCoords.x,
// 			meshData.bIsFlipY ? 1.0f - texCoords.y : texCoords.y);
// 		meshData.AddVertex(vertex);
// 	}
//
// 	for (unsigned int i = 0; i < pMesh->mNumFaces; ++i) {
// 		auto& face = pMesh->mFaces[i];
// 		for (size_t j = 0; j < 3; ++j) {
// 			meshData.AddIndex((unsigned int)face.mIndices[j]);
// 		}
// 	}
//
// 	return true;
// }
//
// static bool s_ProcessMesh(void* pTransform,
// 	struct aiNode* pNode,
// 	const struct aiScene* pScene,
// 	FMeshData& meshData) {
// 	aiMatrix4x4 nodeTransformation = *reinterpret_cast<aiMatrix4x4*>(pTransform)
// 		* pNode->mTransformation;
//
// 	//1> Mesh Data
// 	for (unsigned int i = 0; i < pNode->mNumMeshes; ++i) {
// 		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
// 		if (s_ProcessMeshData(&nodeTransformation, pMesh, pScene, meshData))
// 			return true;
// 	}
//
// 	//2> Nodes Children
// 	for (unsigned int i = 0; i < pNode->mNumChildren; ++i) {
// 		if (s_ProcessMesh(&nodeTransformation, pNode->mChildren[i], pScene,
// 			meshData)) return true;
// 	}
//
// 	return false;
// }
//
//
// std::vector<Mesh> Data::ParseMeshDatas(aiMatrix4x4 transform,
// 	aiScene* scene,
// 	const unsigned int parserFlags) {
// 	const aiMatrix4x4 identity;
// 	aiMatrix4x4 nodeTransformation = identity * scene->mRootNode->
// 		mTransformation;
//
// 	std::vector<Mesh> outMeshes;
// 	for (unsigned int i = 0; i < scene->mRootNode->mNumMeshes; i++) {
// 		aiMesh* mesh = scene->mMeshes[scene->mRootNode->mMeshes[i]];
//
// 		Mesh meshData;
// 		if (s_ProcessMeshData(&nodeTransformation, pMesh, pScene, meshData))
// 			aMeshDatas.push_back(meshData);
// 	}
//
// 	for (unsigned int i = 0; i < pNode->mNumChildren; ++i) {
// 		s_ProcessNode_MeshDatas(&nodeTransformation, pNode->mChildren[i],
// 			pScene, aMeshDatas, isFlipY);
// 	}
// }
//
// static void s_ProcessNode_MeshDatas(void* pTransform, struct aiNode* pNode, const struct aiScene* pScene, FMeshDataVector& aMeshDatas, bool isFlipY)
// {
// 	aiMatrix4x4 nodeTransformation = *reinterpret_cast<aiMatrix4x4*>(pTransform) * pNode->mTransformation;
//
// 	//1> Meshes
// 	for (unsigned int i = 0; i < pNode->mNumMeshes; ++i)
// 	{
// 		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
// 		FMeshData meshData;
// 		meshData.bIsFlipY = isFlipY;
// 		if (s_ProcessMeshData(&nodeTransformation, pMesh, pScene, meshData))
// 			aMeshDatas.push_back(meshData);
// 	}
//
// 	//2> Nodes Children
// 	for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
// 	{
// 		s_ProcessNode_MeshDatas(&nodeTransformation, pNode->mChildren[i], pScene, aMeshDatas, isFlipY);
// 	}
// }
//
// std::vector<Mesh> Data::LoadMeshDatas(const std::string& fbxPath,
// 	const unsigned int parserFlags) {
// 	std::vector<Mesh> outMeshes;
// 	Assimp::Importer import;
//
// 	const UIntegers content = FileUtils::ReadFileAsUIntegers(fbxPath);
// 	const aiScene* scene = import.ReadFileFromMemory(content.data(),
// 		content.size(), parserFlags);
//
// 	if (scene == nullptr) {
// 		throw std::runtime_error("failed to load fbx model!");
// 	}
//
// 	const aiMatrix4x4 identity;
// 	ParseMeshDatas(identity, scene->mRootNode, scene, outMeshes);
// }
