#pragma once

#include "Engine/System/include/BaseObject.h"

class SceneObject: public BaseObject {
protected:
	SceneObject* _parent = nullptr;
	std::vector<SceneObject*> _sons;

	void AddToSons(SceneObject* son) {
		_sons.push_back(son);
	}

public:
	explicit SceneObject(const std::string& root,
		const std::string& file,
		SceneObject*& parent) : BaseObject(root, file), _parent(parent) {
		if (parent != nullptr) {
			parent->AddToSons(this);
		}
		else parent = this;
	}

	~SceneObject() override = default;

	std::vector<SceneObject*> GetSons() {
		return _sons;
	}
};
