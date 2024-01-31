#pragma once

#include <Engine/Light/include/BaseLight.h>

#include <string>
#include <unordered_map>
#include <vector>

class LightChannel : public BaseObject {
  BaseScene* scene = nullptr;
  std::string name = "Unset";
  std::vector<BaseLight*> lights;

 public:
  template <typename... Args>
  explicit LightChannel(const std::string& name, Args&&... args)
      : name(name), BaseObject(std::forward<Args>(args)...) {}
  ~LightChannel() override = default;

  virtual void OnCreate() override;
  void AddLightToChannel(BaseLight* light) { lights.emplace_back(light); }

  const std::string& GetName() { return name; }
  std ::vector<BaseLight*>& GetLights() { return lights; }
};