#pragma once

#include "BaseObject.h"
#include "GraphicsInterface.h"
#include "Engine/Scene/include/BaseScene.h"
#include "Engine/Utility/include/JsonUtils.h"

class Application final: public BaseObject {
	BaseScene* scene = nullptr;
	GraphicsInterface* graphics = nullptr;

	void CreateGraphics();
	void CreateLauncherScene();

public:
	explicit
	Application(const std::string& root, const std::string& file) : BaseObject(
		root, file) {
		OnCreate();
	}

	~Application() override {
		JsonUtils::ClearDocumentCache();

		delete scene;
		delete graphics;
	}

	void OnCreate() override {
		CreateGraphics();
		CreateLauncherScene();
	}

	void Run() const;
};
