#include "../include/Application.h"

#include <Engine/RHI/Vulkan/include/vulkan.h>
#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Scene/include/StartScene.h>
#include <Engine/System/include/GraphicsInterface.h>

void Application::CreateGraphics() {
  const std::string apiPath = JSON_CONFIG(String, "GraphicsConfig");
  if (const std::string rhiType = JsonUtils::ReadStringFromFile(
          GetRoot() + apiPath, "RenderHardwareInterface");
      rhiType == "Vulkan") {
    graphics = Create<Vulkan>(GetRoot(), apiPath);
  } else if (rhiType == "DirectX") {
    throw std::runtime_error("DirectX not supported now!");
  } else {
    throw std::runtime_error(rhiType + " not supported now!");
  }
}

void Application::CreateLauncherScene() {
  const std::string scenePath = JSON_CONFIG(String, "LauncherScene");
  scene = Create<BaseScene>(GetRoot(), scenePath, graphics);
}

void Application::CreateWindow() const {
  graphics->CreateWindow(JSON_CONFIG(String, "ApplicationName"));
  graphics->InitGraphics();
}

void Application::LaunchScene() {
  CreateLauncherScene();
  graphics->RendererLoop();
}

void Application::TerminateScene() const {
  scene->DestroyImmediately();
  graphics->CleanupGraphics();
}

void Application::RunApplication() {
  CreateWindow();
  LaunchScene();
  TerminateScene();
}

void Application::TriggerOnUpdate() {
  for (auto& val : BaseObject::_BaseObjects | std::views::values) {
    auto iter = val.begin();
    while (iter != val.end()) {
      if ((*iter)->_alive == true) {
        BaseObject::BaseObjects[(*iter)->_name].emplace_back(*iter);
        (*iter)->OnStart();
      } else {
        (*iter)->OnDestroy();
        delete *iter;
      }
      iter = val.erase(iter);
    }
  }
  for (auto& val : BaseObject::BaseObjects | std::views::values) {
    auto iter = val.begin();
    while (iter != val.end()) {
      if ((*iter)->_alive == false) {
        (*iter)->OnStop();
        BaseObject::_BaseObjects[(*iter)->_name].emplace_back(*iter);
        iter = val.erase(iter);
      } else {
        (*iter)->OnUpdate();
        iter++;
      }
    }
  }
}

void Application::OnCreate() { CreateGraphics(); }
void Application::OnStart() {}
void Application::OnUpdate() {}
void Application::OnStop() {}
void Application::OnDestroy() {
  graphics->DestroyImmediately();
  JsonUtils::ClearDocumentCache();
}
