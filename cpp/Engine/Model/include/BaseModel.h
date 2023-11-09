#pragma once

#include "BaseMaterial.h"

#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Engine/Utility/include/TypeUtils.h"

#include "Engine/Scene/include/BaseScene.h"
#include "Engine/System/include/BaseObject.h"
#include "Engine/Scene/include/SceneObject.h"

class BaseModel final: public SceneObject {
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
		for (const MeshData* mesh: meshes) {
			delete mesh;
		}
	}

	void OnCreate() override {
		if (JSON_CONFIG(String, "Type") == "FBX") {
			LoadFbxDatas(JSON_CONFIG(String, "File"),
				aiProcess_Triangulate | aiProcess_GenSmoothNormals |
				aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		}
		else if (JSON_CONFIG(String, "Type") == "OBJ") {
			// LoadObjDatas(JSON_CONFIG(String, "File"), 0);
		}
		_scene->GetGraphics()->ParseMeshDatas(meshes);
	}

	// void OnCreate() {
	// 	// scene->draws.emplace_back(device,
	// 	// 	VulkanConfig::SHADER_PATH,
	// 	// 	render.GetRenderPass());
	// 	// draws[0].Load(device,
	// 	// 	render,
	// 	// 	{{VulkanConfig::MODEL_PATH, {VulkanConfig::TEXTURE_PATH}}});
	// }
	//
	// void OnDestroy() {
	// 	// for (const Draw& draw: draws) {
	// 	// 	draw.Destroy(device.GetLogical());
	// 	// }
	// }
};
