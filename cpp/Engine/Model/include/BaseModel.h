#pragma once

#include <Engine/Light/include/LightChannel.h>
#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Scene/include/SceneObject.h>
#include <Engine/System/include/BaseObject.h>
#include <Engine/Utility/include/TypeUtils.h>

class BaseCamera;

class BaseModel final : public SceneObject {
  std::vector<std::shared_ptr<MeshData>> meshes;
  std::weak_ptr<GraphicsInterface> graphics;

  std::string _cameraName = "Unset";
  std::weak_ptr<BaseCamera> _camera;

  std::string _lightChannelName = "Unset";
  std::weak_ptr<LightChannel> _lightChannel;
  virtual void LoadFbxDatas(const unsigned int parserFlags);

 public:
  template <typename... Args>
  explicit BaseModel(std::weak_ptr<GraphicsInterface> graphics, Args&&... args)
      : graphics(graphics), SceneObject(std::forward<Args>(args)...) {}
  ~BaseModel() override = default;

  virtual void OnCreate() override;
  virtual void OnUpdate() override;
  virtual void OnDestroy() override;
  virtual void OnStart() override;
  virtual void OnStop() override;

  virtual void OnActive() override { SceneObject::OnActive(); }
  virtual void OnDeactive() override { SceneObject::OnDeactive(); }

  virtual void SetCamera(const std::string& cameraName);
  virtual void SetLightChannel(const std::string& lightChannelName);

  virtual std::weak_ptr<BaseCamera> GetCamera();
  virtual std::weak_ptr<LightChannel> GetLightChannel();
};
