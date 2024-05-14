#include "../include/Application.h"

#include <Engine/Editor/include/BaseEditor.h>
#include <Engine/Light/include/LightChannel.h>
#include <Engine/Model/include/BaseModel.h>
#include <Engine/RHI/Vulkan/include/vulkan.h>
#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Scene/include/StartScene.h>
#include <Engine/System/include/BaseInput.h>
#include <Engine/System/include/GraphicsInterface.h>

#include <ranges>
#include <thread>

void Application::CreateGraphics() {
  const std::string apiPath = JSON_CONFIG(String, "GraphicsConfig");
  if (const std::string rhiType = JsonUtils::ReadStringFromFile(
          GetRoot() + apiPath, "RenderHardwareInterface");
      rhiType == "Vulkan") {
    graphics = Create<Vulkan>(GetRoot(), apiPath);
  } else if (rhiType == "DirectX") {
    PRINT_AND_THROW_ERROR("DirectX not supported now!");
  } else {
    std::string errorMsg = rhiType + " not supported now!";
    std::cout << errorMsg << std::endl;
    throw std::runtime_error(errorMsg);
  }
}

void Application::CreateWindow() const {
  const std::string configPath = JSON_CONFIG(String, "ApplicationConfig");
  std::string appName =
      JsonUtils::ReadStringFromFile(GetRoot() + configPath, "ApplicationName");
  graphics->CreateWindow(appName);
  graphics->InitGraphics();
}

void Application::CreateLauncherScene() {
  const std::string scenePath = JSON_CONFIG(String, "LauncherScene");
  scene = Create<BaseScene>(graphics, GetRoot(), scenePath);
}

void Application::LaunchScene() {
  if (sceneState == SceneState::Terminated) {
    graphicsSettingsModified = false;
    sceneState = SceneState::Launching;
    if (GetEnableEditor()) {
      std::thread(&Application::StartRenderLoop, this).detach();
    } else {
      StartRenderLoop();
    }
  }
}

void Application::StartRenderLoop() {
  CreateGraphics();
  CreateWindow();
  CreateLauncherScene();
  sceneState = SceneState::Running;
  graphics->RenderLoop();
  sceneState = SceneState::Terminating;

  while (modelResourceManager.processFinished == false) {
  }
  scene->Destroy();
  graphics->CleanupGraphics();
  graphics->Destroy();

  passiveObjects.clear();
  activeObjects.clear();

  graphics.reset();
  scene.reset();

  JsonUtils::ClearDocumentCache();
  graphicsSettingsModified = false;
  sceneState = SceneState::Terminated;
}

void Application::TerminateScene() {
  if (sceneState == SceneState::Running) {
    sceneState = SceneState::Terminating;
    graphics->SetRenderLoopShouldEnd(true);
  }
}

void Application::RunApplication() {
  const std::string configPath = JSON_CONFIG(String, "ApplicationConfig");
  EnableEditor =
      JsonUtils::ReadBoolFromFile(GetRoot() + configPath, "EnableEditor");

  sceneState = SceneState::Terminated;
  if (GetEnableEditor()) {
    CreateEditor();
    editor->LoopImgui();
    TerminateScene();
    while (sceneState != SceneState::Unset &&
           sceneState != SceneState::Terminated) {
    }
    DestroyEditor();
  } else {
    LaunchScene();
  }
}

void Application::TriggerOnUpdate() {
  for (std::shared_ptr<BaseObject> obj : passiveObjects) {
    if (obj->_alive == false) {
      obj->OnDestroy();
    } else if (obj->_active) {
      obj->OnStart();
      activeObjects.emplace_back(obj);
    } else {
      obj->OnDeactive();
      obj->_locked = false;
    }
  }
  passiveObjects.clear();

  auto iter = activeObjects.begin();
  while (iter != activeObjects.end()) {
    if ((*iter)->_alive && (*iter)->_active) {
      (*iter)->OnUpdate();
      iter++;
    } else {
      (*iter)->OnStop();
      passiveObjects.emplace_back(*iter);
      iter = activeObjects.erase(iter);
    }
  }
}

std::weak_ptr<SceneObject> Application::GetSceneRootObject() {
  if (scene) {
    return scene->GetRootObject();
  }
  return std::shared_ptr<SceneObject>(nullptr);
}

void Application::OnCreate() { BaseObject::OnCreate(); }
void Application::OnStart() { BaseObject::OnStart(); }
void Application::OnUpdate() { BaseObject::OnUpdate(); }
void Application::OnStop() { BaseObject::OnStop(); }
void Application::OnDestroy() {
  JsonUtils::ClearDocumentCache();
  BaseObject::OnDestroy();
}

std::unordered_map<int, std::weak_ptr<BaseLight>>&
Application::GetLightsById() {
  return scene->GetLightsById();
}
std::weak_ptr<LightChannel> Application::GetLightChannelByName(
    const std::string& name) const {
  return scene->GetLightChannelByName(name);
}

void Application::CreateEditor() {
  std::cout << "Load Editor GUI\n";
  editor = Create<BaseEditor>(GetRoot(), JSON_CONFIG(String, "EditorConfig"));
  editor->LoadImgui();
}

void Application::UpdateEditor() { editor->UpdateImgui(); }

void Application::DestroyEditor() {
  std::cout << "Destroy Editor GUI\n";
  editor->DestroyImgui();
}

int Application::GetGraphicsWindowWidth() { return graphics->GetWindowWidth(); }
int Application::GetGraphicsWindowHeight() {
  return graphics->GetWindowHeight();
}

bool& Application::GetShowRenderFrame() {
  return graphics->GetShowRenderFrame();
}
bool& Application::GetShowGameFrame() { return graphics->GetShowGameFrame(); }

uint32_t Application::GetRenderFrameCount() const {
  return graphics->GetRenderFrameCount();
}
uint32_t Application::GetGameFrameCount() const {
  return graphics->GetGameFrameCount();
}

int Application::GetMSAASamples() const {
  if (graphics) {
    return graphics->GetMSAASamples();
  }
  std::string graphicsConfigPath = JSON_CONFIG(String, "GraphicsConfig");
  return JsonUtils::ReadIntFromFile(GetRoot() + graphicsConfigPath,
                                    "MSAAMaxSamples");
}
bool Application::GetEnableMipmap() const {
  if (graphics) {
    return graphics->GetEnableMipmap();
  }
  std::string graphicsConfigPath = JSON_CONFIG(String, "GraphicsConfig");
  return JsonUtils::ReadBoolFromFile(GetRoot() + graphicsConfigPath,
                                     "EnableMipmap");
}
bool Application::GetEnableZPrePass() const {
  if (graphics) {
    return graphics->GetEnableZPrePass();
  }
  std::string graphicsConfigPath = JSON_CONFIG(String, "GraphicsConfig");
  return JsonUtils::ReadBoolFromFile(GetRoot() + graphicsConfigPath,
                                     "EnableZPrePass");
}
bool Application::GetEnableShadowMap() const {
  if (graphics) {
    return graphics->GetEnableShadowMap();
  }
  std::string graphicsConfigPath = JSON_CONFIG(String, "GraphicsConfig");
  return JsonUtils::ReadBoolFromFile(GetRoot() + graphicsConfigPath,
                                     "EnableShadowMap");
}
bool Application::GetEnableDeferred() const {
  if (graphics) {
    return graphics->GetEnableDeferred();
  }
  std::string graphicsConfigPath = JSON_CONFIG(String, "GraphicsConfig");
  return JsonUtils::ReadBoolFromFile(GetRoot() + graphicsConfigPath,
                                     "EnableDeferred");
}
bool Application::GetEnableShaderDebug() const {
  if (graphics) {
    return graphics->GetEnableShaderDebug();
  }
  std::string graphicsConfigPath = JSON_CONFIG(String, "GraphicsConfig");
  return JsonUtils::ReadBoolFromFile(GetRoot() + graphicsConfigPath,
                                     "EnableShaderDebug");
}