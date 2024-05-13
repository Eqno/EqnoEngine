#pragma once

#include <Engine/Editor/include/BaseEditor.h>
#include <Engine/System/include/BaseObject.h>
#include <Engine/System/include/BaseResource.h>
#include <Engine/Utility/include/JsonUtils.h>

#include <memory>
#include <ranges>

class BaseScene;
class GraphicsInterface;
enum class SceneState { Unset, Launching, Running, Terminating, Terminated };

class Application final : public BaseObject {
  friend class BaseObject;
  BaseResource modelResourceManager;

  std::list<std::shared_ptr<BaseObject>> passiveObjects;
  std::list<std::shared_ptr<BaseObject>> activeObjects;

  std::shared_ptr<BaseScene> scene;
  std::shared_ptr<BaseEditor> editor;
  std::shared_ptr<GraphicsInterface> graphics;

  bool graphicsSettingsModified = false;
  SceneState sceneState = SceneState::Unset;
  void CreateGraphics();
  void CreateWindow() const;
  void CreateLauncherScene();

 public:
  template <typename... Args>
  explicit Application(Args&&... args)
      : BaseObject(std::forward<Args>(args)...,
                   std::shared_ptr<BaseObject>(nullptr)) {}
  virtual ~Application() = default;
  std::unordered_map<int, std::weak_ptr<BaseLight>>& GetLightsById();
  std::weak_ptr<GraphicsInterface> GetGraphics() const { return graphics; }
  std::weak_ptr<BaseScene> GetScene() const { return scene; }

  void TriggerOnUpdate();
  void RunApplication();

  virtual void OnCreate() override;
  virtual void OnStart() override;
  virtual void OnUpdate() override;
  virtual void OnStop() override;
  virtual void OnDestroy() override;

  std::weak_ptr<LightChannel> GetLightChannelByName(
      const std::string& name) const;

  void CreateEditor();
  void UpdateEditor();
  void DestroyEditor();

  void LaunchScene();
  void StartRenderLoop();
  void TerminateScene();

  GLFWwindow* GetParentWindow() const { return editor->GetParentWindow(); }
  GLFWwindow* GetEditorWindow() const { return editor->GetWindow(); }
  int GetGraphicsWindowWidth();
  int GetGraphicsWindowHeight();

  SceneState GetSceneState() const { return sceneState; }
  virtual bool GetGraphicsSettingsModified() const {
    return graphicsSettingsModified;
  }
  virtual void SetGraphicsSettingsModified(bool modified) {
    graphicsSettingsModified = modified;
  }
  void AddModelToResourceWaitQueue(std::function<void()> func,
                                   std::shared_ptr<BaseObject> obj) {
    modelResourceManager.AddToWaitQueue(func, obj);
  }
  std::weak_ptr<SceneObject> GetSceneRootObject();

 private:
  bool EnableEditor = false;

 public:
  bool GetEnableEditor() { return EnableEditor; }
  bool GetLaunchSceneInEditor() { return editor->GetLaunchSceneInEditor(); }

  bool& GetShowRenderFrame();
  bool& GetShowGameFrame();

  uint32_t GetRenderFrameCount() const;
  uint32_t GetGameFrameCount() const;

  int GetMSAASamples() const;
  bool GetEnableMipmap() const;
  bool GetEnableZPrePass() const;
  bool GetEnableShadowMap() const;
  bool GetEnableDeferred() const;
  bool GetEnableShaderDebug() const;
};