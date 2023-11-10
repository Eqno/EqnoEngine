#pragma once

#include "SceneObject.h"
#include "Engine/Model/include/BaseMaterial.h"
#include "Engine/System/include/BaseObject.h"
#include "Engine/System/include/GraphicsInterface.h"

class BaseScene final: public BaseObject {
	SceneObject* rootObject = nullptr;
	GraphicsInterface* graphics = nullptr;
	std::unordered_map<std::string, BaseMaterial*> materials;

public:
	BaseMaterial* GetMaterialByPath(const std::string& path) {
		if (!materials.contains(path)) {
			materials[path] = new BaseMaterial(GetRoot(), path);
		}
		return materials[path];
	}

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
