#pragma once

#include <Engine/Light/include/BaseLight.h>

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

class LightChannel : public BaseObject {
  std::weak_ptr<BaseScene> scene;
  std::string name = "Unset";
  std::list<std::weak_ptr<BaseLight>> lights;

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
  std::function<BaseLight*()> GetLights() {
    auto lightsIter = lights.begin();
    return [&, lightsIter]() mutable -> BaseLight* {
      while (lightsIter != lights.end()) {
        if (auto lightPtr = lightsIter->lock()) {
          auto ret = lightPtr.get();
          lightsIter++;
          return ret;
        } else {
          lightsIter = lights.erase(lightsIter);
        }
      }
      return nullptr;
    };
  }
};