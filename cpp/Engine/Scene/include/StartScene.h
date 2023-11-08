#pragma once

#include "BaseScene.h"
#include "SceneObject.h"
#include "Engine/Utility/include/JsonUtils.h"

class StartScene final: public BaseScene {
	SceneObject* rootObject = nullptr;

public:
	explicit
	StartScene(const std::string& root, const std::string& file) : BaseScene(
		root, file) {
		OnCreate();
	}

	~StartScene() override {
		std::cout << "Destroy" << std::endl;
		DestroyObjects(rootObject);
	}

	static void DestroyObjects(SceneObject* root) {
		for (SceneObject* son: root->GetSons()) {
			DestroyObjects(son);
			delete son;
		}
	}

	void OnCreate() override {
		JsonUtils::ParseSceneObjectTree(GetRoot() + GetFile(), GetRoot(),
			rootObject);
	}
};
