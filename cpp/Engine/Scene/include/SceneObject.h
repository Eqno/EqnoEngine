#pragma once

#include <Engine/Model/include/BaseTransform.h>
#include <Engine/System/include/BaseObject.h>

class BaseScene;

class SceneObject : public BaseObject {
 protected:
  BaseScene* scene = nullptr;
  SceneObject* parent = nullptr;

  std::string name;
  std::list<SceneObject*> sons;

  BaseTransform transform;
  void AddToSons(SceneObject* son) { sons.push_back(son); }

 public:
  template <typename... Args>
  explicit SceneObject(SceneObject*& parent, std::string name, Args&&... args)
      : parent(parent), name(name), BaseObject(std::forward<Args>(args)...) {
    if (parent != nullptr) {
      parent->AddToSons(this);
    } else {
      parent = this;
    }
  }
  ~SceneObject() override = default;

  std::list<SceneObject*>& GetSons() { return sons; }
  [[nodiscard]] SceneObject* GetParent() const { return parent; }
  [[nodiscard]] BaseTransform& GetTransform() { return transform; }

  virtual void UpdateAbsoluteTransform();

  virtual void SetRelativePosition(const glm::vec3& position);
  virtual void SetRelativeRotation(const glm::vec3& rotation);
  virtual void SetRelativeScale(const glm::vec3& scale);

  virtual void PrintSons();
  virtual glm::mat4x4 GetAbsoluteTransform();

  virtual void OnCreate() override;
  virtual void OnUpdate() override;
};
