#include "../include/Application.h"

#include <Engine/RHI/Vulkan/include/vulkan.h>
#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Scene/include/StartScene.h>
#include <Engine/System/include/GraphicsInterface.h>

#include <ranges>

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
  scene = Create<BaseScene>(graphics, GetRoot(), scenePath);
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
  scene->Destroy();
  graphics->CleanupGraphics();
}

void Application::RunApplication() {
  CreateWindow();
  LaunchScene();
  TerminateScene();
}

void Application::TriggerOnUpdate() {
  for (BaseObject* obj : BaseObject::PassiveObjects) {
    if (obj->_alive == false) {
      obj->OnDestroy();
      delete obj;
    } else if (obj->_active) {
      obj->OnStart();
      BaseObject::ActiveObjects.emplace_back(obj);
    } else {
      obj->OnDeactive();
      obj->_locked = false;
    }
  }
  BaseObject::PassiveObjects.clear();

  auto iter = BaseObject::ActiveObjects.begin();
  while (iter != BaseObject::ActiveObjects.end()) {
    if ((*iter)->_alive && (*iter)->_active) {
      (*iter)->OnUpdate();
      iter++;
    } else {
      (*iter)->OnStop();
      BaseObject::PassiveObjects.emplace_back(*iter);
      iter = BaseObject::ActiveObjects.erase(iter);
    }
  }
}

void Application::OnCreate() {
  BaseObject::OnCreate();
  CreateGraphics();
}
void Application::OnStart() { BaseObject::OnStart(); }
void Application::OnUpdate() { BaseObject::OnUpdate(); }
void Application::OnStop() { BaseObject::OnStop(); }
void Application::OnDestroy() {
  BaseObject::OnDestroy();

  graphics->Destroy();
  JsonUtils::ClearDocumentCache();
}
