#include "../include/BaseScene.h"

#include <ranges>

void DestroyObjects(SceneObject* root) {
  for (SceneObject* son : root->GetSons()) {
    DestroyObjects(son);
    son->DestroyImmediately();
  }
}

void BaseScene::OnCreate() {
  BaseObject::OnCreate();
  rootObject =
      Create<SceneObject>(rootObject, "RootObject", GetRoot(), GetFile());
  JsonUtils::ParseSceneObjectTree(graphics, rootObject, GetRoot() + GetFile(),
                                  GetRoot(), this);
}

void BaseScene::OnDestroy() {
  BaseObject::OnDestroy();

  DestroyObjects(rootObject);
  rootObject->DestroyImmediately();

  for (BaseMaterial* material : materials | std::views::values) {
    material->DestroyImmediately();
  }
}
