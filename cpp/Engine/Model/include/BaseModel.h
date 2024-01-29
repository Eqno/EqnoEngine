#pragma once

#include <Engine/Light/include/LightChannel.h>
#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Scene/include/SceneObject.h>
#include <Engine/System/include/BaseObject.h>
#include <Engine/Utility/include/TypeUtils.h>

class BaseCamera;

class BaseModel final : public SceneObject {
  std::vector<MeshData*> meshes;
  GraphicsInterface* graphics;

  std::string _cameraName = "Unset";
  BaseCamera* _camera = nullptr;

  std::string _lightChannelName = "Unset";
  LightChannel* _lightChannel = nullptr;

  virtual void LoadFbxDatas(const std::string& fbxPath,
                            const unsigned int parserFlags);

 public:
  template <typename... Args>
  explicit BaseModel(GraphicsInterface* graphics, Args&&... args)
      : graphics(graphics), SceneObject(std::forward<Args>(args)...) {}
  ~BaseModel() override = default;

  virtual void OnCreate() override;
  virtual void OnUpdate() override;
  virtual void OnDestroy() override;
  virtual void OnStart() override;
  virtual void OnStop() override { SceneObject::OnStop(); }

  virtual void OnActive() override { SceneObject::OnActive(); }
  virtual void OnDeactive() override { SceneObject::OnDeactive(); }

  virtual void SetCamera(const std::string& cameraName);
  virtual void SetLightChannel(const std::string& lightChannelName);

  virtual BaseCamera* GetCamera();
  virtual LightChannel* GetLightChannel();
};
