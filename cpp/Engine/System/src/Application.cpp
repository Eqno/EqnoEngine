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
  scene->DestroyImmediately();
  graphics->CleanupGraphics();
}

void Application::RunApplication() {
  CreateWindow();
  LaunchScene();
  TerminateScene();
}

void Application::TriggerOnUpdate() {
  {
    auto& _map = BaseObject::_BaseObjects;
    auto mapIter = _map.begin();
    while (mapIter != _map.end()) {
      auto& _list = mapIter->second;
      auto listIter = _list.begin();
      while (listIter != _list.end()) {
        if ((*listIter)->_alive == true) {
          BaseObject::BaseObjects[(*listIter)->_name].emplace_back(*listIter);
          (*listIter)->OnStart();
        } else {
          (*listIter)->OnDestroy();
          delete *listIter;
        }
        listIter = _list.erase(listIter);
      }
      mapIter = _map.erase(mapIter);
    }
  }
  {
    auto& _map = BaseObject::BaseObjects;
    auto mapIter = _map.begin();
    while (mapIter != _map.end()) {
      auto& _list = mapIter->second;
      auto listIter = _list.begin();
      while (listIter != _list.end()) {
        if ((*listIter)->_alive == false) {
          (*listIter)->OnStop();
          BaseObject::_BaseObjects[(*listIter)->_name].emplace_back(*listIter);
          listIter = _list.erase(listIter);
        } else {
          (*listIter)->OnUpdate();
          listIter++;
        }
      }
      if (_list.empty()) {
        mapIter = _map.erase(mapIter);
      } else {
        mapIter++;
      }
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

  graphics->DestroyImmediately();
  JsonUtils::ClearDocumentCache();
}
