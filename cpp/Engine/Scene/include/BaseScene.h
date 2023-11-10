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
		OnDestroy();
	}

	void OnCreate() override;
	void OnDestroy() override;

	[[nodiscard]] GraphicsInterface* GetGraphics() const {
		return graphics;
	}
};
