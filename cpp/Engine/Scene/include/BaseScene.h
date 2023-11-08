#pragma once

#include "Engine/System/include/BaseObject.h"

class BaseScene: public BaseObject {
public:
	explicit BaseScene(const std::string& name,
		const std::string& root,
		const std::string& file) : BaseObject(name, root, file) {}

	~BaseScene() override = default;
};
