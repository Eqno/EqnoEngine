#pragma once

#include "JsonUtils.h"
#include "BaseObject.h"
#include "GraphicsInterface.h"
#include "Engine/Scene/include/BaseScene.h"

class Application final: public BaseObject {
	BaseScene* scene = nullptr;
	GraphicsInterface* graphics = nullptr;

	void CreateGraphics();
	void CreateLauncherScene();

public:
	explicit Application(const std::string& name,
		const std::string& root,
		const std::string& file) : BaseObject(name, root, file) {
		CreateGraphics();
		CreateLauncherScene();
	}

	~Application() override {
		JsonUtils::ClearDocumentCache();

		delete scene;
		delete graphics;
	}
	void Run() const;
};
