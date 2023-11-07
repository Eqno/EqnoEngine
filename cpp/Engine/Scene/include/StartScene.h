#pragma once

#include "BaseScene.h"

class StartScene: public BaseScene {
	std::vector<Draw> draws;

public:
	explicit StartScene(const std::string& path) {
		name = "Scene";
		RegisterToObjects(name, this);

		BaseModel model(this, ModelConfig::MODEL_PATH + "TestModel");
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
