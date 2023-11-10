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

	void CreateWindow() const;
	void LaunchScene();
	void TerminateScene() const;

public:
	explicit
	Application(const std::string& root, const std::string& file) : BaseObject(
		root, file) {
		OnCreate();
	}

	~Application() override {
		OnDestroy();
	}

	void OnCreate() override;
	void OnDestroy() override;

	void RunApplication();
};
