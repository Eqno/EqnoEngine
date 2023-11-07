#pragma once

#include "Engine/System/include/BaseObject.h"

class BaseModel final : public BaseObject {
	// const Base* scene;

public:
	explicit BaseModel(const BaseObject* scene, const std::string& path) {
		name = "Model";
		RegisterToObjects(name, this);
	}

	void OnCreate() override {
		// scene->draws.emplace_back(device,
		// 	VulkanConfig::SHADER_PATH,
		// 	render.GetRenderPass());
		// draws[0].Load(device,
		// 	render,
		// 	{{VulkanConfig::MODEL_PATH, {VulkanConfig::TEXTURE_PATH}}});
	}

	void OnStart() override {}
	void OnUpdate() override {}
	void OnStop() override {}

	void OnDestroy() override {
		// for (const Draw& draw: draws) {
		// 	draw.Destroy(device.GetLogical());
		// }
	}
};
