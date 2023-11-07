#pragma once

#include "FileUtils.h"
#include "GraphicsInterface.h"
#include "Engine/Scene/include/BaseScene.h"

class Application {
	inline static std::string gameRoot = "Unset";

	BaseScene* scene = nullptr;
	GraphicsInterface* graphics = nullptr;

public:
	explicit Application(const std::string& rootPath) {
		gameRoot = rootPath + "/";
		FileUtils::AddGameFilePath();
		FileUtils::GetGameFilePath("");
	}

	static std::string& GetGameRoot() {
		return gameRoot;
	}
};
