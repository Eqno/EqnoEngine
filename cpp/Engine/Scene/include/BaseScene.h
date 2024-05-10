#pragma once

#include <Engine/System/include/BaseObject.h>

class BaseLight;
class BaseModel;
class BaseCamera;
class SceneObject;
class LightChannel;
class BaseMaterial;
class GraphicsInterface;

struct aiMaterial;

class BaseScene final : public BaseObject {
  std::shared_ptr<SceneObject> rootObject;
  std::weak_ptr<GraphicsInterface> graphics;
  std::unordered_map<int, std::weak_ptr<BaseLight>> lightsById;

  std::unordered_map<std::string, std::shared_ptr<BaseMaterial>> materials;
  std::unordered_map<std::string, std::shared_ptr<BaseCamera>> cameras;
  std::unordered_map<std::string, std::shared_ptr<BaseLight>> lights;
  std::unordered_map<std::string, std::shared_ptr<LightChannel>> lightChannels;

 public:
  int GetAvailableLightId();
  std::weak_ptr<BaseLight> GetLightById(int id);
  std::unordered_map<int, std::weak_ptr<BaseLight>>& GetLightsById() {
    return lightsById;
  }

  std::weak_ptr<BaseMaterial> GetMaterialByPath(const std::string& path,
                                                aiMaterial* matData);
  std::weak_ptr<BaseCamera> GetCameraByName(const std::string& name);
  const std::unordered_map<std::string, std::shared_ptr<BaseLight>>& GetLights()
      const;
  std::weak_ptr<BaseLight> GetLightByName(const std::string& name);
  std::weak_ptr<LightChannel> GetLightChannelByName(const std::string& name);

  void RegisterCamera(const std::string& name,
                      std::shared_ptr<BaseCamera> camera);
  int RegisterLight(const std::string& name, std::shared_ptr<BaseLight> light);
  void RegisterLightChannel(const std::string& name,
                            std::shared_ptr<LightChannel> channel);

  void UnregisterMaterial(const std::string& name);
  void UnregisterCamera(const std::string& name);
  void UnregisterLight(const std::string& name);
  void UnregisterLightChannel(const std::string& name);

  template <typename... Args>
  explicit BaseScene(std::weak_ptr<GraphicsInterface> graphics, Args&&... args)
      : graphics(graphics), BaseObject(std::forward<Args>(args)...) {}
  ~BaseScene() override = default;

  [[nodiscard]] std::weak_ptr<GraphicsInterface> GetGraphics() const {
    return graphics;
  }
  void AddModelToResourceWaitQueue(std::function<void()> func,
                                   std::shared_ptr<BaseObject> obj);
  std::weak_ptr<SceneObject> GetRootObject() { return rootObject; }

  virtual void OnCreate() override;
  virtual void OnStart() override;
  virtual void OnUpdate() override { BaseObject::OnUpdate(); }
  virtual void OnStop() override { BaseObject::OnStop(); }
  virtual void OnDestroy() override;
};
