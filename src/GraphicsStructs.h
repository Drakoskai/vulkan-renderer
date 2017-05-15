#pragma once

#include "GraphicsEnums.h"

class Mesh;

class __declspec(novtable) IRenderer {
public:
	virtual ~IRenderer() { }
	virtual void Init() = 0;
	virtual void PrepareFrame() = 0;
	virtual void DrawFrame() = 0;
	virtual void EndFrame() = 0;
	virtual void InitMesh(Mesh* mesh) = 0;
};

struct Viewport {
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
};

struct PipeLineContext {
	BlendMode blendMode;
	CullMode cullMode;
	DepthFunction depthFunction;
	FillMode fillMode;
	PrimitiveTopology primitiveTopology;
};
