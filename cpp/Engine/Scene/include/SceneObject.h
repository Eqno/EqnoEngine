#pragma once

#include <Engine/Model/include/BaseTransform.h>
#include <Engine/System/include/BaseObject.h>

class BaseScene;

class SceneObject : public BaseObject {
 protected:
  std::weak_ptr<BaseScene> scene;
  std::weak_ptr<SceneObject> parent;

  std::string name;
  std::list<std::shared_ptr<SceneObject>> sons;

  BaseTransform transform;
  void AddToSons(std::shared_ptr<SceneObject> son) { sons.push_back(son); }

 public:
  template <typename... Args>
  explicit SceneObject(std::weak_ptr<SceneObject> parent, std::string name,
                       Args&&... args)
      : parent(parent), name(name), BaseObject(std::forward<Args>(args)...) {}
  ~SceneObject() override = default;

  std::list<std::shared_ptr<SceneObject>>& GetSons() { return sons; }
  [[nodiscard]] std::weak_ptr<SceneObject> GetParent() const { return parent; }
  [[nodiscard]] BaseTransform& GetTransform() { return transform; }

  virtual void UpdateAbsoluteTransform();

  virtual void SetRelativePosition(const glm::vec3& position);
  virtual void SetRelativeRotation(const glm::vec3& rotation);
  virtual void SetRelativeScale(const glm::vec3& scale);

  virtual void PrintSons();
  virtual glm::mat4x4& GetAbsoluteTransform();
  virtual glm::vec3& GetAbsolutePosition();
  virtual glm::vec3& GetAbsoluteForward();

  virtual void OnCreate() override;
  virtual void OnDestroy() override;
};
