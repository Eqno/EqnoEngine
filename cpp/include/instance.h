#pragma once
#include "validation.h"

class Instance {
	VkInstance instance = nullptr;

public:
	[[nodiscard]] const VkInstance& GetVkInstance() const {
		return instance;
	}

	void CreateInstance(const Validation& validation);
	void DestroyInstance() const;
};
