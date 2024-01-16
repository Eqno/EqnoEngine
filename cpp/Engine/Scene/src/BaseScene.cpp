#include "../include/BaseScene.h"

#include <ranges>

void DestroyObjects(SceneObject* root) {
  for (SceneObject* son : root->GetSons()) {
    DestroyObjects(son);
    son->Destroy();
  }
}

void BaseScene::OnCreate() {
  BaseObject::OnCreate();
  JsonUtils::ParseSceneObjectTree(graphics, rootObject, this,
                                  GetRoot() + GetFile(), GetRoot());
}

void BaseScene::OnDestroy() {
  BaseObject::OnDestroy();
  DestroyObjects(rootObject);
  for (const BaseMaterial* material : materials | std::views::values) {
    delete material;
  }
}
