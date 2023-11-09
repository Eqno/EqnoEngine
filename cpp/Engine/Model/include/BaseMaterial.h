#pragma once
#include "Engine/System/include/BaseObject.h"

class BaseMaterial: public BaseObject {
public:
	explicit BaseMaterial(const std::string& root,
		const std::string& file) : BaseObject(root, file) {}
};
