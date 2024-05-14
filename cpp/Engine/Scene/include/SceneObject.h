#pragma once

#include <Engine/Model/include/BaseTransform.h>
#include <Engine/System/include/BaseObject.h>

#include <mutex>

class BaseScene;

class SceneObject : public BaseObject {
 protected:
  std::weak_ptr<BaseScene> scene;
  std::weak_ptr<SceneObject> parent;

  std::string name;
  std::list<std::shared_ptr<SceneObject>> sons;
  BaseTransform transform;

 public:
  template <typename... Args>
  explicit SceneObject(std::weak_ptr<SceneObject> parent, std::string name,
                       Args&&... args)
      : parent(parent), name(name), BaseObject(std::forward<Args>(args)...) {}
  ~SceneObject() override = default;

  const std::string& GetName() { return name; }
  std::list<std::shared_ptr<SceneObject>>& GetSons() { return sons; }
  [[nodiscard]] std::weak_ptr<SceneObject> GetParent() const { return parent; }
  [[nodiscard]] BaseTransform& GetTransform() { return transform; }

  virtual void PrintSons();
  virtual void AddToSons(std::shared_ptr<SceneObject> son) {
    sons.push_back(son);
  }
  virtual void RemoveFromSons(std::weak_ptr<SceneObject> son) {
    if (auto sonPtr = son.lock()) {
      auto iter = sons.begin();
      while (iter != sons.end()) {
        if (iter->get() == sonPtr.get()) {
          sons.erase(iter);
          break;
        }
        iter++;
      }
    }
  }

  void UpdateRelativeTransform() { transform.UpdateRelativeTransform(); }
  void UpdateAbsoluteTransform() { transform.UpdateAbsoluteTransform(); }

  const glm::mat4x4& GetRelativeTransform() const {
    return transform.GetRelativeTransform();
  }
  const glm::mat4x4& GetAbsoluteTransform() const {
    return transform.GetAbsoluteTransform();
  }
  const glm::vec3& GetRelativePosition() const {
    return transform.GetRelativePosition();
  }
  const glm::vec3& GetAbsolutePosition() const {
    return transform.GetAbsolutePosition();
  }
  const glm::vec3& GetRelativeRotation() const {
    return transform.GetRelativeRotation();
  }
  const glm::vec3& GetAbsoluteRotation() const {
    return transform.GetAbsoluteRotation();
  }
  const glm::vec3& GetRelativeScale() const {
    return transform.GetRelativeScale();
  }
  const glm::vec3& GetAbsoluteScale() const {
    return transform.GetAbsoluteScale();
  }

  const glm::vec3& GetRelativeForward() {
    return transform.GetRelativeForward();
  }
  const glm::vec3& GetRelativeLeft() { return transform.GetRelativeLeft(); }
  const glm::vec3& GetRelativeUp() { return transform.GetRelativeUp(); }
  const glm::vec3& GetAbsoluteForward() {
    return transform.GetAbsoluteForward();
  }
  const glm::vec3& GetAbsoluteLeft() { return transform.GetAbsoluteLeft(); }
  const glm::vec3& GetAbsoluteUp() { return transform.GetAbsoluteUp(); }

  void SetRelativePosition(const glm::vec3& pos) {
    transform.SetRelativePosition(pos);
  }
  void SetAbsolutePosition(const glm::vec3& pos) {
    transform.SetAbsolutePosition(pos);
  }
  void SetRelativeRotation(const glm::vec3& rot) {
    transform.SetRelativeRotation(rot);
  }
  void SetAbsoluteRotation(const glm::vec3& rot) {
    transform.SetAbsoluteRotation(rot);
  }
  void SetRelativeScale(const glm::vec3& scale) {
    transform.SetRelativeScale(scale);
  }
  void SetAbsoluteScale(const glm::vec3& scale) {
    transform.SetAbsoluteScale(scale);
  }

  virtual void OnCreate() override;
  virtual void OnDestroy() override;
};
