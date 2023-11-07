#pragma once

#include <string>
#include <unordered_map>

class BaseObject {
protected:
	std::string name = "Unset";

public:
	virtual ~BaseObject() = default;

	virtual void OnCreate() = 0;
	virtual void OnStart() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnStop() = 0;
	virtual void OnDestroy() = 0;

	static std::unordered_map<std::string, std::vector<BaseObject*>>
	BaseObjects;

	static void RegisterToObjects(const std::string& name, BaseObject* inst) {
		BaseObjects[name].push_back(inst);
	}
};
