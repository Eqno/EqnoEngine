#pragma once

#include <Engine/System/include/BaseObject.h>
#include <Engine/Utility/include/JsonUtils.h>

#include <ranges>

class BaseScene;
class GraphicsInterface;

class Application final : public BaseObject {
  friend class BaseObject;

  std::list<std::shared_ptr<BaseObject>> passiveObjects;
  std::list<std::shared_ptr<BaseObject>> activeObjects;

  std::shared_ptr<BaseScene> scene;
  std::shared_ptr<GraphicsInterface> graphics;

  void CreateGraphics();
  void CreateLauncherScene();

  void CreateWindow() const;
  void LaunchScene();
  void TerminateScene() const;

 public:
  template <typename... Args>
  explicit Application(Args&&... args)
      : BaseObject(std::forward<Args>(args)...,
                   std::shared_ptr<BaseObject>(nullptr)) {
    this->_app = static_pointer_cast<Application>(shared_from_this());
  }
  ~Application() override = default;
  std::unordered_map<int, std::weak_ptr<BaseLight>>& GetLightsById();

  void TriggerOnUpdate();
  void RunApplication();

  virtual void OnCreate() override;
  virtual void OnStart() override;
  virtual void OnUpdate() override;
  virtual void OnStop() override;
  virtual void OnDestroy() override;
};
