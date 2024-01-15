#pragma once

#include "Engine/Model/include/BaseTransform.h"
#include "Engine/System/include/BaseObject.h"

class SceneObject : public BaseObject {
 protected:
  SceneObject* _parent = nullptr;
  std::vector<SceneObject*> _sons;

  BaseTransform transform;

  void AddToSons(SceneObject* son) { _sons.push_back(son); }

 public:
  template <typename... Args>
  explicit SceneObject(SceneObject*& parent, Args&&... args)
      : _parent(parent), BaseObject(std::forward<Args>(args)...) {
    if (parent != nullptr) {
      parent->AddToSons(this);
    } else {
      parent = this;
    }
  }
  ~SceneObject() override = default;

  std::vector<SceneObject*> GetSons() { return _sons; }

  [[nodiscard]] SceneObject* GetParent() const { return _parent; }

  [[nodiscard]] BaseTransform& GetTransform() { return transform; }

  virtual void OnCreate() override;
};
