#pragma once
#pragma once
#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include <Engine/System/include/BaseObject.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

enum class LastSelectType { None, File, Object };
class Application;
class BaseEditor : public BaseObject {
 private:
  ImGuiIO* ioPtr = nullptr;
  GLFWwindow* parentWindow = nullptr;
  GLFWwindow* window = nullptr;
  ImGui_ImplVulkanH_Window* wd = nullptr;

  LastSelectType lastSelectType = LastSelectType::None;
  std::pair<std::string, std::weak_ptr<rapidjson::Document>> selectedFile{
      "Unset", std::shared_ptr<rapidjson::Document>(nullptr)};
  std::pair<std::string, std::weak_ptr<SceneObject>> selectedObject{
      "Unset", std::shared_ptr<SceneObject>(nullptr)};

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  float editorWindowWidth = 1280;
  float editorWindowHeight = 720;

  float graphicsWindowWidth = 1280;
  float graphicsWindowHeight = 720;
  std::string windowTitle = "Unset";

  int msaaSamples = 0;
  bool enableMipmap = false;
  bool enableZPrePass = false;
  bool enableShadowMap = false;
  bool enableDeferredRendering = false;

  bool showFileExplorer = true;
  bool showSceneHierarchy = false;
  bool showObjectInspector = false;

  void EditorDrawMenuBar();
  void EditorDrawRestartTip();
  void EditorDrawFrameCount();
  void EditorDrawLaunchCommand();
  void EditorDrawFileExplorer();
  void EditorDrawSceneHierarchy();
  void EditorDrawObjectInspector();

  bool expandAllFiles = false;
  bool collapseAllFiles = false;
  bool onlyShowEngineFiles = true;

  bool expandAllObjects = true;
  bool collapseAllObjects = false;

  bool expandAllProperties = true;
  bool collapseAllProperties = false;

  bool CheckDoubleClick();
  std::string ParseFilePath(std::string path);
  std::string GetObjectName(const std::string& originName,
                            std::weak_ptr<SceneObject> self,
                            std::weak_ptr<SceneObject> parent);

  bool GetAddToSceneEnable();

  std::weak_ptr<rapidjson::Document> LoadJsonFile(
      const std::filesystem::path& path);
  void DisplayJson(rapidjson::Value& value, bool& modifiedValue);
  void DisplayDirectory(const std::filesystem::path& path);
  void DisplayHierarchy(std::weak_ptr<SceneObject> root, std::string path);
  void DisplayProperties(std::weak_ptr<SceneObject> object);

 public:
  template <typename... Args>
  explicit BaseEditor(Args&&... args)
      : BaseObject(std::forward<Args>(args)...) {}
  ~BaseEditor() override = default;

  Application* appPointer = nullptr;
  void GetAppPointer();

  void LoadImgui();
  void LoopImgui();
  void UpdateImgui();
  void DestroyImgui();

  GLFWwindow* GetParentWindow() const { return parentWindow; }
  GLFWwindow* GetWindow() const { return window; }
  bool GetShowRestartTip();

  int GetGraphicsWindowWidth() const { return graphicsWindowWidth; }
  int GetGraphicsWindowHeight() const { return graphicsWindowWidth; }

  int GetEditorWindowWidth() const { return editorWindowWidth; }
  int GetEditorWindowHeight() const { return editorWindowHeight; }

 private:
  void InitConfig();
  bool LaunchSceneInEditor = false;

 public:
  bool GetLaunchSceneInEditor() const { return LaunchSceneInEditor; }
};