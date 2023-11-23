#pragma once

#include <glm/mat4x4.hpp>

class SceneObject;

#define TRANSFORM_MEMBERS(where) \
	where(vec3, Position, 0.0f) \
	where(vec3, Rotation, 0.0f) \
	where(vec3, Scale, 0.0f) \
	where(vec3, Right, (1.0f, 0.0f, 0.0f)) \
	where(vec3, Up, (0.0f, 1.0f, 0.0f)) \
	where(vec3, Forward, (0.0f, 0.0f, 1.0f)) \
	where(mat4x4, Transform, 1)

#define DEFINE_GETTER(class, type, upper) \
	glm::type class Get##upper()

#define IMPLEMENT_GETTER(upper, lower) { \
		if ((lower).first == true) { \
			Update##upper(); \
		} \
		return (lower).second; \
	}

#define DEFINE_SETTER(class, type, member, upper) \
	void class Set##upper##member(const glm::type& (member))

#define IMPLEMENT_SETTER(member, lower) { \
		lower##member = {false, member}; \
		DirtAbsolute##member(); \
	}

#define DEFINE_DIRTTER(class, upper) \
	void class Dirt##upper()

#define IMPLEMENT_DIRTTER(upper, lower, opposite) { \
		Update##opposite(); \
		(lower).first = true; \
		for (SceneObject* son : _owner->GetSons()) { \
			son->GetTransform().Dirt##upper(); \
		} \
	}

#define DEFINE_MEMBER(type, member, upper, lower, value) \
private: \
	std::pair<bool, glm::type> lower##member = {false, glm::type(value)}; \
public: \
	DEFINE_GETTER(, type, upper##member); \
	DEFINE_SETTER(, type, member, upper); \
	DEFINE_DIRTTER(, upper##member);

#define IMPLEMENT_MEMBER(type, member, upper, lower, opposite) \
	DEFINE_GETTER(BaseTransform::, type, upper##member) \
	IMPLEMENT_GETTER(upper##member, lower##member) \
	DEFINE_SETTER(BaseTransform::, type, member, upper) \
	IMPLEMENT_SETTER(member, lower) \
	DEFINE_DIRTTER(BaseTransform::, upper##member) \
	IMPLEMENT_DIRTTER(upper##member, lower##member, opposite##member)

#define DEFINE_HEADER(type, member, value) \
	DEFINE_MEMBER(type, member, Relative, relative, value) \
	DEFINE_MEMBER(type, member, Absolute, absolute, value)

#define IMPLEMENT_SOURCE(type, member, value) \
	IMPLEMENT_MEMBER(type, member, Relative, relative, Absolute) \
	IMPLEMENT_MEMBER(type, member, Absolute, absolute, Relative)

class BaseTransform {
	TRANSFORM_MEMBERS(DEFINE_HEADER)

private:
	SceneObject* _owner = nullptr;

	void UpdateRelativePosition();
	void UpdateAbsolutePosition();

	void UpdateRelativeRotation();
	void UpdateAbsoluteRotation();

	void UpdateRelativeScale();
	void UpdateAbsoluteScale();

	void UpdateRelativeRight();
	void UpdateAbsoluteRight();

	void UpdateRelativeUp();
	void UpdateAbsoluteUp();

	void UpdateRelativeForward();
	void UpdateAbsoluteForward();

	void UpdateRelativeTransform();
	void UpdateAbsoluteTransform();

public:
	void RegisterOwner(SceneObject* other) {
		_owner = other;
	}
};
