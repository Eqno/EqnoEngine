#pragma once

#include "BaseObject.h"

struct MeshData;

class GraphicsInterface: public BaseObject {
public:
	explicit
	GraphicsInterface(const std::string& root,
		const std::string& file) : BaseObject(root, file) {}

	~GraphicsInterface() override = default;

	virtual void CreateWindow(const std::string& title) = 0;
	virtual void InitGraphics() = 0;
	virtual void RendererLoop() = 0;
	virtual void CleanupGraphics() = 0;

	virtual void ParseMeshDatas(std::vector<MeshData*>& meshDatas) = 0;
};
