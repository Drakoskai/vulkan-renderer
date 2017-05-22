#pragma once

#include "GraphicsEnums.h"

class Mesh;

class __declspec(novtable) IRenderer {
public:
	virtual ~IRenderer() {}
	virtual void Init() = 0;
	virtual void PrepareFrame() = 0;
	virtual void PresentFrame() = 0;
	virtual void EndFrame() = 0;
	virtual void InitMesh(Mesh* mesh) = 0;
};

static const std::string DefaultVertexShader = "shaders/default_vert.spv";
static const std::string DefaultFragmentShader = "shaders/default_frag.spv";

struct ShaderId {
private:
	std::string vertexshader;
	std::string fragmentshader;
public:
	size_t vertHash = 0;
	size_t fragHash = 0;
	ShaderId() : vertexshader(DefaultVertexShader), fragmentshader(DefaultFragmentShader) {
		vertHash = std::hash<std::string>()(vertexshader);
		fragHash = std::hash<std::string>()(fragmentshader);
	}

	ShaderId(std::string vertshader, std::string fragshader) : vertexshader(vertshader), fragmentshader(fragshader) {
		vertHash = std::hash<std::string>()(vertexshader);
		fragHash = std::hash<std::string>()(fragmentshader);
	}

	size_t HashCode() const {
		return vertHash ^ fragHash << 1;
	}

	void SetVertexShader(std::string vertshader) {
		vertexshader = vertshader;
		vertHash = std::hash<std::string>()(vertexshader);
	}

	void SetFragmentShader(std::string fragshader) {
		fragmentshader = fragshader;
		fragHash = std::hash<std::string>()(fragshader);
	}
	const std::string& GetVertexShader()const {
		return vertexshader;
	}

	const std::string& GetFragmentShader()const {
		return fragmentshader;
	}

	bool operator==(const ShaderId& other) const {
		return HashCode() == other.HashCode();
	}

	bool operator>(const ShaderId& other) const {
		return HashCode() > other.HashCode();
	}

	bool operator<(const ShaderId& other) const {
		return HashCode() < other.HashCode();
	}
};

namespace std {
	template<> struct hash<ShaderId> {
		size_t operator()(ShaderId const& id) const {
			return id.HashCode();
		}
	};
}

struct TextureId {
private:
	std::string texturename;

public:
	size_t texturenamehash = 0;
	TextureId() {}

	TextureId(std::string texturefile) :texturename(texturefile) {
		texturenamehash = std::hash<std::string>()(texturename);
	}

	size_t HashCode() const {
		return texturenamehash;
	}

	void SetTextureName(std::string textname) {
		texturename = textname;
		texturenamehash = std::hash<std::string>()(texturename);
	}
	const std::string& GetTextureName()const {
		return texturename;
	}

	bool operator==(const TextureId& other) const {
		return HashCode() == other.HashCode();
	}

	bool operator>(const TextureId& other) const {
		return HashCode() > other.HashCode();
	}

	bool operator<(const TextureId& other) const {
		return HashCode() < other.HashCode();
	}
};

namespace std {
	template<> struct hash<TextureId> {
		size_t operator()(TextureId const& id) const {
			return id.HashCode();
		}
	};
}

struct Viewport {
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
};

struct PipelineContext {
	BlendMode blendMode;
	CullMode cullMode;
	DepthFunction depthFunction;
	FillMode fillMode;
	PrimitiveTopology primitiveTopology;
};
