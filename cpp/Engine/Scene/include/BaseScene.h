#pragma once

#include <Engine/System/include/BaseObject.h>

class BaseLight;
class BaseCamera;
class SceneObject;
class LightChannel;
class BaseMaterial;
class GraphicsInterface;

class BaseScene final : public BaseObject {
  SceneObject* rootObject = nullptr;
  GraphicsInterface* graphics = nullptr;

  std::unordered_map<std::string, BaseMaterial*> materials;
  std::unordered_map<std::string, BaseCamera*> cameras;
  std::unordered_map<std::string, BaseLight*> lights;
  std::unordered_map<std::string, LightChannel*> lightChannels;

 public:
  BaseMaterial* GetMaterialByPath(const std::string& path);
  BaseCamera* GetCameraByName(const std::string& name);
  BaseLight* GetLightByName(const std::string& name);
  LightChannel* GetLightChannelByName(const std::string& name);

  void RegisterCamera(const std::string& name, BaseCamera* camera);
  void RegisterLight(const std::string& name, BaseLight* light);
  void RegisterLightChannel(const std::string& name, LightChannel* channel);

  template <typename... Args>
  explicit BaseScene(GraphicsInterface* graphics, Args&&... args)
      : graphics(graphics), BaseObject(std::forward<Args>(args)...) {}
  ~BaseScene() override = default;

  [[nodiscard]] GraphicsInterface* GetGraphics() const { return graphics; }

  virtual void OnCreate() override;
  virtual void OnStart() override;
  virtual void OnUpdate() override { BaseObject::OnUpdate(); }
  virtual void OnStop() override { BaseObject::OnStop(); }
  virtual void OnDestroy() override;
};
