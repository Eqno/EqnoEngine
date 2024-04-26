#include "../include/BaseScene.h"

#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Light/include/BaseLight.h>
#include <Engine/Light/include/LightChannel.h>
#include <Engine/Model/include/BaseMaterial.h>
#include <Engine/Scene/include/SceneObject.h>
#include <Engine/System/include/BaseObject.h>
#include <assimp/types.h>

#include <ranges>

void BaseScene::OnCreate() {
  BaseObject::OnCreate();

  rootObject =
      Create<SceneObject>(rootObject, "RootObject", GetRoot(), GetFile());
  JsonUtils::ParseSceneObjectTree(graphics, rootObject, GetRoot(), GetFile(),
                                  shared_from_this());
  JsonUtils::ParseSceneLightChannels(
      GetRoot(), GetFile(),
      std::static_pointer_cast<BaseScene>(shared_from_this()));
}

void BaseScene::OnStart() {
  BaseObject::OnStart();
  puts("--- Scene Model ---");
  rootObject->PrintSons();
  puts("--- Model Loaded ---");
}

void BaseScene::OnDestroy() {
  BaseObject::OnDestroy();

  rootObject->Destroy();
  for (std::shared_ptr<BaseMaterial> material :
       materials | std::views::values) {
    material->Destroy();
  }
  for (std::shared_ptr<LightChannel> lightChannel :
       lightChannels | std::views::values) {
    lightChannel->Destroy();
  }
}

std::weak_ptr<BaseMaterial> BaseScene::GetMaterialByPath(
    const std::string& path, aiMaterial* matData) {
  std::string name = path + ": " + matData->GetName().C_Str();
  if (materials.contains(name) == false) {
    materials[name] =
        Create<BaseMaterial>(matData, name, false, GetRoot(), path);
  }
  return materials[name];
}
std::weak_ptr<BaseCamera> BaseScene::GetCameraByName(const std::string& name) {
  if (cameras.contains(name)) {
    return cameras[name];
  }
  return std::shared_ptr<BaseCamera>(nullptr);
}
const std::unordered_map<std::string, std::shared_ptr<BaseLight>>&
BaseScene::GetLights() const {
  return lights;
}
std::weak_ptr<BaseLight> BaseScene::GetLightByName(const std::string& name) {
  if (lights.contains(name)) {
    return lights[name];
  }
  return std::shared_ptr<BaseLight>(nullptr);
}
std::weak_ptr<BaseLight> BaseScene::GetLightById(int id) {
  if (lightsById.contains(id)) {
    return lightsById[id];
  }
  return std::shared_ptr<BaseLight>(nullptr);
}
std::weak_ptr<LightChannel> BaseScene::GetLightChannelByName(
    const std::string& name) {
  if (lightChannels.contains(name)) {
    return lightChannels[name];
  }
  return std::shared_ptr<LightChannel>(nullptr);
}
void BaseScene::RegisterCamera(const std::string& name,
                               std::shared_ptr<BaseCamera> camera) {
  cameras[name] = camera;
}
int BaseScene::GetAvailableLightId() {
  for (int i = 0; i < MaxLightNum; i++) {
    auto iter = lightsById.find(i);
    if (iter == lightsById.end() || !iter->second.lock()) {
      return i;
    }
  }
  PRINT_AND_THROW_ERROR("light id exceed max num!");
}
int BaseScene::RegisterLight(const std::string& name,
                             std::shared_ptr<BaseLight> light) {
  lights[name] = light;
  int id = GetAvailableLightId();
  lightsById[id] = lights[name];
  return id;
}
void BaseScene::RegisterLightChannel(const std::string& name,
                                     std::shared_ptr<LightChannel> channel) {
  lightChannels[name] = channel;
}
void BaseScene::UnregisterMaterial(const std::string& name) {
  materials.erase(name);
}
void BaseScene::UnregisterCamera(const std::string& name) {
  cameras.erase(name);
}
void BaseScene::UnregisterLight(const std::string& name) { lights.erase(name); }
void BaseScene::UnregisterLightChannel(const std::string& name) {
  lightChannels.erase(name);
}

void BaseScene::AddModelToResourceWaitQueue(std::function<void()> func,
                                            std::shared_ptr<BaseObject> obj) {
  modelResourceManager.AddToWaitQueue(func, obj);
}