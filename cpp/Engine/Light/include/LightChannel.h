#pragma once

#include <Engine/Light/include/BaseLight.h>

#include <string>
#include <unordered_map>
#include <vector>

class LightChannel : public BaseObject {
  std::weak_ptr<BaseScene> scene;
  std::string name = "Unset";
  std::vector<std::weak_ptr<BaseLight>> lights;

 public:
  template <typename... Args>
  explicit LightChannel(const std::string& name, Args&&... args)
      : name(name), BaseObject(std::forward<Args>(args)...) {}
  ~LightChannel() override = default;

  virtual void OnCreate() override;
  virtual void OnDestroy() override;
  void AddLightToChannel(std::weak_ptr<BaseLight> light) {
    lights.emplace_back(light);
  }

  const std::string& GetName() { return name; }
  std ::vector<std::weak_ptr<BaseLight>>& GetLights() { return lights; }
};