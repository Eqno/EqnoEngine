#pragma once

#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Engine/RHI/Vulkan/deps/glm/glm/mat4x4.hpp"
#include "Engine/Utility/include/TypeUtils.h"

#include "Engine/Scene/include/BaseScene.h"
#include "Engine/System/include/BaseObject.h"
#include "Engine/Scene/include/SceneObject.h"

class BaseModel final : public SceneObject {
	glm::mat4x4 transform = glm::mat4x4(1);
	std::vector<MeshData*> meshes;

	static void ParseFbxData(const aiMatrix4x4& transform,
		const aiMesh* mesh,
		MeshData* meshData);

	void ParseFbxDatas(const aiMatrix4x4& transform,
		const aiNode* node,
		const aiScene* scene);

	void LoadFbxDatas(const std::string& fbxPath, unsigned parserFlags);

public:
	explicit BaseModel(const std::string& root,
		const std::string& file,
		SceneObject*& parent,
		BaseScene* inScene) : SceneObject(root, file, parent, inScene) {
		OnCreate();
	}

	~BaseModel() override {
		OnDestroy();
	}

	void OnCreate() override;
	void OnDestroy() override;
};
