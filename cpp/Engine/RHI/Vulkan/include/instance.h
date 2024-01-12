#pragma once

#include <vulkan/vulkan_core.h>

#include "base.h"

class Validation;

class Instance : public Base {
  VkInstance instance = nullptr;

 public:
  [[nodiscard]] const VkInstance& GetVkInstance() const { return instance; }

  void CreateInstance(const Validation& validation);
  void DestroyInstance() const;
};
