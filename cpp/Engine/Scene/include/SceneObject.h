#pragma once

#include "Engine/Model/include/BaseTransform.h"
#include "Engine/System/include/BaseObject.h"

class SceneObject: public BaseObject {
protected:
	BaseScene* _scene = nullptr;
	SceneObject* _parent = nullptr;
	std::vector<SceneObject*> _sons;

	BaseTransform transform;

	void AddToSons(SceneObject* son) {
		_sons.push_back(son);
	}

public:
	explicit SceneObject(const std::string& root,
		const std::string& file,
		SceneObject*& parent,
		BaseScene* inScene) : BaseObject(root, file),
		_scene(inScene),
		_parent(parent) {
		if (parent != nullptr) {
			parent->AddToSons(this);
		}
		else parent = this;
	}

	~SceneObject() override = default;

	std::vector<SceneObject*> GetSons() {
		return _sons;
	}

	[[nodiscard]] SceneObject* GetParent() const {
		return _parent;
	}

	[[nodiscard]] const BaseTransform& GetTransform() const {
		return transform;
	}

	void OnCreate() override;
};
