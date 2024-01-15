#pragma once

#include <Engine/System/include/BaseObject.h>
#include <Engine/Utility/include/JsonUtils.h>

#include <ranges>

class BaseScene;
class GraphicsInterface;

class Application final : public BaseObject {
  BaseScene* scene = nullptr;
  GraphicsInterface* graphics = nullptr;

  void CreateGraphics();
  void CreateLauncherScene();

  void CreateWindow() const;
  void LaunchScene();
  void TerminateScene() const;

 public:
  template <typename... Args>
  explicit Application(Args&&... args)
      : BaseObject(std::forward<Args>(args)..., nullptr) {}
  ~Application() override = default;

  void TriggerOnUpdate();
  void RunApplication();

  virtual void OnCreate() override;
  virtual void OnStart() override;
  virtual void OnUpdate() override;
  virtual void OnStop() override;
  virtual void OnDestroy() override;
};
