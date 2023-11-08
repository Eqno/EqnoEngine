#pragma once

#include "BaseScene.h"

class StartScene final: public BaseScene {
public:
	explicit StartScene(const std::string& name,
		const std::string& root,
		const std::string& file) : BaseScene(name, root, file) {}

	~StartScene() override = default;
};
