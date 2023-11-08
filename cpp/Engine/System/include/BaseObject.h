#pragma once

#include <string>
#include <unordered_map>

class BaseObject {
protected:
	static std::unordered_map<std::string, std::vector<BaseObject*>>
	BaseObjects;

	std::string _name = "Unset";
	std::string _root = "Unset";
	std::string _file = "Unset";

	explicit BaseObject(const std::string& name,
		std::string root,
		std::string file) : _name(name),
		_root(std::move(root)),
		_file(std::move(file)) {
		BaseObjects[name].push_back(this);
	}

public:
	BaseObject() = delete;
	virtual ~BaseObject() = default;

	BaseObject& operator=(const BaseObject& app) = delete;
	BaseObject& operator=(BaseObject&& app) = delete;

	BaseObject(const BaseObject& app) = delete;
	BaseObject(BaseObject&& app) = delete;

	virtual void OnCreate();
	virtual void OnStart();
	virtual void OnUpdate();
	virtual void OnStop();
	virtual void OnDestroy();

	[[nodiscard]] const std::string& GetName() const {
		return _name;
	}

	[[nodiscard]] const std::string& GetRoot() const {
		return _root;
	}

	[[nodiscard]] const std::string& GetFile() const {
		return _file;
	}

	static const BaseObject* GetObjectByName(const std::string& name) {
		const auto& objects = BaseObjects[name];
		return objects.empty() ? nullptr : objects.back();
	}

	[[nodiscard]] static const std::vector<BaseObject*>& GetObjectsByName(
		const std::string& name) {
		return BaseObjects[name];
	}
};