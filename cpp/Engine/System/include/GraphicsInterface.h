#pragma once

class GraphicsInterface {
public:
	virtual ~GraphicsInterface() = default;

	GraphicsInterface() = delete;
	GraphicsInterface(const GraphicsInterface& other) = delete;
	GraphicsInterface(GraphicsInterface&& other) = delete;

	GraphicsInterface& operator =(const GraphicsInterface& other) = delete;
	GraphicsInterface& operator =(GraphicsInterface&& other) = delete;

	virtual void CreateWindow() = 0;
	virtual void InitGraphics() = 0;
	virtual void RendererLoop() = 0;
	virtual void CleanupGraphics() = 0;
};
