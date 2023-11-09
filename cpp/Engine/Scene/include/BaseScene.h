#pragma once

#include "SceneObject.h"
#include "Engine/System/include/BaseObject.h"
#include "Engine/System/include/GraphicsInterface.h"

class BaseScene final: public BaseObject {
	SceneObject* rootObject = nullptr;
	GraphicsInterface* graphics = nullptr;

public:
	explicit BaseScene(const std::string& root,
		const std::string& file,
		GraphicsInterface* graphics) : BaseObject(root, file),
		graphics(graphics) {
		OnCreate();
	}

	~BaseScene() override {
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
			rootObject, this);
	}

	[[nodiscard]] GraphicsInterface* GetGraphics() const {
		return graphics;
	}
};
