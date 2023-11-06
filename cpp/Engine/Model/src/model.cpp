#include "../include/model.h"



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

// Data ParseMeshData(const aiMatrix4x4& transform, const aiMesh* mesh) {
// 	Data data;
// 	if (mesh->mNumVertices <= 0) return data;
//
// 	data.SetName(mesh->mName.C_Str());
// 	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
// 		const aiVector3D pos = transform * mesh->mVertices[i];
// 		const aiColor4D color = mesh->mColors[0] ? mesh->mColors[0][i]
// 			: aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
// 		const aiVector3D normal = transform * (mesh->mNormals
// 			? mesh->mNormals[i]
// 			: aiVector3D(0.0f, 0.0f, 1.0f));
// 		const aiVector3D tangent = mesh->mTangents
// 			? transform * mesh->mTangents[i] : aiVector3D(1.0f, 0.0f, 0.0f);
// 		const aiVector3D texCoord = mesh->mTextureCoords[0]
// 			? mesh->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);
//
// 		vertices.emplace_back(MathUtils::AiVector3D2GlmVec3(pos),
// 			MathUtils::AiColor4D2GlmVec4(color),
// 			MathUtils::AiVector3D2GlmVec3(normal),
// 			MathUtils::AiVector3D2GlmVec3(tangent),
// 			MathUtils::AiVector2D2GlmVec2(texCoord));
// 	}
//
// 	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
// 		const auto& face = mesh->mFaces[i];
// 		for (size_t j = 0; j < 3; ++j) {
// 			indices.emplace_back(face.mIndices[j]);
// 		}
// 	}
// 	return data;
// }
//
//
// void Data::ParseMeshDatas(const aiMatrix4x4& transform,
// 	const aiNode* node,
// 	const aiScene* scene,
// 	std::vector<Mesh>& outMeshes) {
// 	const aiMatrix4x4 nodeTransform = transform * node->mTransformation;
//
// 	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
// 		const aiMesh* pMesh = scene->mMeshes[node->mMeshes[i]];
// 		outMeshes.push_back(ParseMeshData(nodeTransform, pMesh, scene));
// 	}
//
// 	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
// 		ParseMeshDatas(nodeTransform, node->mChildren[i], scene, outMeshes);
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
// 		content.size(),
// 		parserFlags);
//
// 	if (scene == nullptr) {
// 		throw std::runtime_error("failed to load fbx model!");
// 	}
//
// 	const aiMatrix4x4 identity;
// 	ParseMeshDatas(identity, scene->mRootNode, scene, outMeshes);
// }
