#pragma once

#include "Engine/Scene/include/SceneObject.h"

class BaseModel final: public SceneObject {
public:
	explicit BaseModel(const std::string& root,
		const std::string& file,
		SceneObject*& parent) : SceneObject(root, file, parent) {
		OnCreate();
	}

	void OnCreate() override {
		// scene->draws.emplace_back(device,
		// 	VulkanConfig::SHADER_PATH,
		// 	render.GetRenderPass());
		// draws[0].Load(device,
		// 	render,
		// 	{{VulkanConfig::MODEL_PATH, {VulkanConfig::TEXTURE_PATH}}});
	}

	void OnDestroy() override {
		// for (const Draw& draw: draws) {
		// 	draw.Destroy(device.GetLogical());
		// }
	}
};
