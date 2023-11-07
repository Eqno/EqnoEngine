#pragma once

#include "JsonUtils.h"
#include "BaseObject.h"
#include "GraphicsInterface.h"
#include "Engine/Scene/include/BaseScene.h"

class Application: public BaseObject {
	inline static std::string gameRoot = "Unset";

	BaseScene* scene = nullptr;
	GraphicsInterface* graphics = nullptr;

public:
	explicit Application(const std::string& rootPath) : BaseObject() {
		gameRoot = rootPath + "/";
		CreateGraphics();
	}

	~Application() override {
		JsonUtils::ClearDocumentCache();

		delete scene;
		delete graphics;
	}

	static std::string& GetGameRoot() {
		return gameRoot;
	}

	void CreateGraphics();

	void OnCreate() override {}
	void OnStart() override {}
	void OnUpdate() override {}
	void OnStop() override {}
	void OnDestroy() override {}
};
