#include <Engine/Model/include/BaseMaterial.h>
#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Utility/include/MathUtils.h>
#include <Engine/Utility/include/TypeUtils.h>

void BaseMaterial::OnCreate() {
  BaseObject::OnCreate();

  JsonUtils::ParseMaterialShaders(GetRoot() + GetFile(), shaders);
  if (matData != nullptr) {
    aiColor4D color;
    matData->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    this->color = MathUtils::AiColor4D2GlmVec4(color);
    matData->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
    matData->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
  }
  JsonUtils::ParseMaterialParams(GetRoot() + GetFile(), color, roughness,
                                 metallic);
}
void BaseMaterial::OnDestroy() {
  BaseObject::OnDestroy();

  if (auto ownerPtr = GetOwner().lock()) {
    scene = std::dynamic_pointer_cast<BaseScene>(ownerPtr);
    if (auto scenePtr = scene.lock()) {
      scenePtr->UnregisterMaterial(name);
    }
  }
}