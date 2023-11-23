#include "../include/SceneObject.h"

void SceneObject::OnCreate() {
	BaseObject::OnCreate();
	transform.RegisterOwner(this);

	glm::mat4 trans = glm::mat4x4(1);
	trans =	glm::scale(trans, glm::vec3(2, 1, 0.5));
	// trans = glm::rotate(trans, glm::radians(30.0f), glm::vec3(0, 1, 0));
	trans = glm::translate(trans, glm::vec3(5, 6, 3));
	glm::vec3 scale = glm::vec3(glm::length(trans[0]), glm::length(trans[1]), glm::length(trans[2]));
	glm::vec3 rotation = glm::degrees(eulerAngles(glm::quat(trans)));
	glm::vec3 translation = trans[3];
	trans;
}