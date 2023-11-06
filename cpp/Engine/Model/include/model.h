#pragma once

#include <assimp/matrix4x4.h>

#include "Engine/RHI/Vulkan/include/config.h"
#include "Engine/RHI/Vulkan/include/draw.h"
#include "Engine/RHI/Vulkan/include/utils.h"
#include "Engine/System/include/base.h"

class Model final : public Base {
	// const Base* scene;

public:
	explicit Model(const Base* scene, const std::string& path) {
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
