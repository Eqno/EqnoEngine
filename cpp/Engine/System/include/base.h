#pragma once

#include <string>
#include <unordered_map>

class Base {
protected:
	std::string name = "Unset";

public:
	virtual ~Base() = default;

	virtual void OnCreate() = 0;
	virtual void OnStart() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnStop() = 0;
	virtual void OnDestroy() = 0;

	static std::unordered_map<std::string, std::vector<Base*>> Objects;

	static void RegisterToObjects(const std::string& name, Base* inst) {
		Objects[name].push_back(inst);
	}
};

