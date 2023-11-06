#pragma once

#include <string>

#include "Engine/Model/include/config.h"
#include "Engine/Model/include/model.h"
#include "Engine/System/include/base.h"

class Scene final : public Base {
	std::vector<Draw> draws;

public:
	explicit Scene(const std::string& path) {
		name = "Scene";
		RegisterToObjects(name, this);

		Model model(this, ModelConfig::MODEL_PATH + "TestModel");
	}

	std::vector<Draw>& GetDraws() {
		return draws;
	}

	// void AddToDraws(Draw draw) {
	// 	draws.emplace_back(draw);
	// }

	void OnCreate() override {}
	void OnStart() override {}
	void OnUpdate() override {}
	void OnStop() override {}
	void OnDestroy() override {}
};
