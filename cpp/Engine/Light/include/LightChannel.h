#pragma once

#include <Engine/Light/include/BaseLight.h>

#include <string>
#include <unordered_map>
#include <vector>

class LightChannel {

  std::string name;
  std::vector<BaseLight*> lights;

 public:
  void AddLightToChannel(BaseLight* light) { lights.emplace_back(light); }
};