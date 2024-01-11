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
  explicit Application(const std::string& root, const std::string& file)
      : BaseObject(nullptr, root, file) {}
  ~Application() override {}
  void TriggerOnUpdate();
  void RunApplication();

  void OnCreate() override;
  void OnStart() override;
  void OnUpdate() override;
  void OnStop() override;
  void OnDestroy() override;
};
