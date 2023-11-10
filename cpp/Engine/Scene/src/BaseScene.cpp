#include "../include/BaseScene.h"

#include <ranges>

void DestroyObjects(SceneObject* root) {
	for (SceneObject* son: root->GetSons()) {
		DestroyObjects(son);
		delete son;
	}
}

void BaseScene::OnCreate() {
	BaseObject::OnCreate();
	JsonUtils::ParseSceneObjectTree(GetRoot() + GetFile(), GetRoot(),
		rootObject, this);
}

void BaseScene::OnDestroy() {
	BaseObject::OnDestroy();
	DestroyObjects(rootObject);
	for (const BaseMaterial* material: materials | std::views::values) {
		delete material;
	}
}
