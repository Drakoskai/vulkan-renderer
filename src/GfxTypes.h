#ifndef GFX_TYPES_H_
#define GFX_TYPES_H_
#include "Util.h"

enum PipelineStateType {
	ClearFlagsColor = 1 << 0,
	ClearFlagsDepth = 1 << 1,
	ClearFlagsDontClear = 1 << 2,
	ClearFlags = ClearFlagsColor | ClearFlagsDepth | ClearFlagsDontClear,

	BlendModeAlphaBlend = 1 << 3,
	BlendModeAdditive = 1 << 4,
	BlendModeOff = 1 << 5,
	BlendMode = BlendModeAlphaBlend | BlendModeAdditive | BlendModeOff,

	DepthFunctionLessOrEqualWriteOff = 1 << 6,
	DepthFunctionLessOrEqualWriteOn = 1 << 7,
	DepthFunctionNoneWriteOff = 1 << 8,
	DepthFunction = DepthFunctionLessOrEqualWriteOff | DepthFunctionLessOrEqualWriteOn | DepthFunctionNoneWriteOff,

	TextureWrapRepeat = 1 << 9,
	TextureWrapClamp = 1 << 10,
	TextureWrap = TextureWrapRepeat | TextureWrapClamp,

	TextureFilterNearest = 1 << 11,
	TextureFilterLinear = 1 << 12,
	TextureFilter = TextureFilterNearest | TextureFilterLinear,

	MipmapsGenerate = 1 << 13,
	MipmapsNone = 1 << 14,
	Mipmaps = MipmapsGenerate | MipmapsNone,

	ColorSpaceRGB = 1 << 15,
	ColorSpaceSRGB = 1 << 16,
	ColorSpace = ColorSpaceRGB | ColorSpaceSRGB,

	Anisotropyk1 = 1 << 17,
	Anisotropyk2 = 1 << 18,
	Anisotropyk4 = 1 << 19,
	Anisotropyk8 = 1 << 20,
	Anisotropyk16 = 1 << 21,
	Anisotropy = Anisotropyk1 | Anisotropyk2 | Anisotropyk4 | Anisotropyk8 | Anisotropyk16,

	CullModeBack = 1 << 22,
	CullModeFront = 1 << 23,
	CullModeOff = 1 << 24,
	CullMode = CullModeBack | CullModeFront | CullModeOff,

	FillModeSolid = 1 << 25,
	FillModeWireWireframe = 1 << 26,
	FillMode = FillModeSolid | FillModeWireWireframe,

	PrimitiveTopologyTriangles = 1 << 27,
	PrimitiveTopologyLines = 1 << 28,
	PrimitiveTopologyPatches = 1 << 29,
	PrimitiveTopologyPoints = 1 << 30,
	PrimitiveTopology = PrimitiveTopologyTriangles | PrimitiveTopologyLines | PrimitiveTopologyPatches | PrimitiveTopologyPoints
};

struct PipelineStateBuilder {
	uint64_t pipelineState = 0L;

	void SetClearFlags(PipelineStateType flags) {
		if (flags & ClearFlags) {
			pipelineState = pipelineState | flags;
		}
	}

	void SetBlendMode(PipelineStateType flags) {
		if (flags & BlendMode) {
			pipelineState = pipelineState | flags;
		}
	}

	void SetDepthFunction(PipelineStateType flags) {
		if (flags & DepthFunction) {
			pipelineState = pipelineState | flags;
		}
	}

	void SetTextureWrap(PipelineStateType flags) {
		if (flags & TextureWrap) {
			pipelineState = pipelineState | flags;
		}
	}

	void SetTextureFilter(PipelineStateType flags) {
		if (flags & TextureFilter) {
			pipelineState = pipelineState | flags;
		}
	}

	void SetMipmaps(PipelineStateType flags) {
		if (flags & Mipmaps) {
			pipelineState = pipelineState | flags;
		}
	}

	void SetColorSpace(PipelineStateType flags) {
		if (flags & ColorSpace) {
			pipelineState = pipelineState | flags;
		}
	}

	void SetAnisotropy(PipelineStateType flags) {
		if (flags & Anisotropy) {
			pipelineState = pipelineState | flags;
		}
	}

	void SetCullMode(PipelineStateType flags) {
		if (flags & CullMode) {
			pipelineState = pipelineState | flags;
		}
	}

	void SetFillMode(PipelineStateType flags) {
		if (flags & FillMode) {
			pipelineState = pipelineState | flags;
		}
	}

	void SetPrimitiveTopology(PipelineStateType flags) {
		if (flags & PrimitiveTopology) {
			pipelineState = pipelineState | flags;
		}
	}
};

inline void PrintPipelineState(const uint64_t pipelineState) {
	switch (pipelineState & ClearFlags) {
	case ClearFlagsColor:
		DebugPrintF("Clear Flags: Color");
		break;
	case ClearFlagsDepth:
		DebugPrintF("Clear Flags: Depth");
		break;
	case ClearFlagsDontClear:
		DebugPrintF("Clear Flags: Don't");
		break;
	default:
		DebugPrintF("Clear Flags: NA");
		break;
	}

	switch (pipelineState & BlendMode) {
	case BlendModeAlphaBlend:
		DebugPrintF("Blend Mode: AlphaBlend");
		break;
	case BlendModeAdditive:
		DebugPrintF("Blend Mode: Additive");
		break;
	case BlendModeOff:
		DebugPrintF("Blend Mode: Off");
		break;
	default:
		DebugPrintF("Blend Mode: NA");
		break;
	}

	switch (pipelineState & DepthFunction) {
	case DepthFunctionLessOrEqualWriteOff:
		DebugPrintF("Depth Function: Less Or Equal Write Off");
		break;
	case DepthFunctionLessOrEqualWriteOn:
		DebugPrintF("Depth Function: Less Or Equal Write On");
		break;
	case DepthFunctionNoneWriteOff:
		DebugPrintF("Depth Function: None Write Off");
		break;
	default:
		DebugPrintF("Depth Function: NA");
		break;
	}

	switch (pipelineState & TextureWrap) {
	case TextureWrapRepeat:
		DebugPrintF("Texture Wrap: Repeat");
		break;
	case TextureWrapClamp:
		DebugPrintF("Texture Wrap: Clamp");
		break;
	default:
		DebugPrintF("Texture Wrap: NA");
		break;
	}

	switch (pipelineState & TextureFilter) {
	case TextureFilterNearest:
		DebugPrintF("Texture Filter: Nearest");
		break;
	case TextureFilterLinear:
		DebugPrintF("Texture Filter: Linear");
		break;
	default:
		DebugPrintF("Texture Filter: NA");
		break;
	}

	switch (pipelineState & Mipmaps) {
	case MipmapsGenerate:
		DebugPrintF("Mipmaps: Generate");
		break;
	case MipmapsNone:
		DebugPrintF("Mipmaps: None");
		break;
	default:
		DebugPrintF("Mipmaps: NA");
		break;
	}

	switch (pipelineState & ColorSpace) {
	case ColorSpaceRGB:
		DebugPrintF("Color Space: RGB");
		break;
	case ColorSpaceSRGB:
		DebugPrintF("Color Space: SRGB");
		break;
	default:
		DebugPrintF("Color Space: NA");
		break;
	}

	switch (pipelineState & Anisotropy) {
	case Anisotropyk1:
		DebugPrintF("Anisotropy: 1");
		break;
	case Anisotropyk2:
		DebugPrintF("Anisotropy: 2");
		break;
	case Anisotropyk4:
		DebugPrintF("Anisotropy: 4");
		break;
	case Anisotropyk8:
		DebugPrintF("Anisotropy: 8");
		break;
	case Anisotropyk16:
		DebugPrintF("Anisotropy: 16");
		break;
	default:
		DebugPrintF("Anisotropy: NA");
		break;
	}

	switch (pipelineState & CullMode) {
	case CullModeBack:
		DebugPrintF("Culling Mode: Back");
		break;
	case CullModeFront:
		DebugPrintF("Culling Mode: Front");
		break;
	case CullModeOff:
		DebugPrintF("Culling Mode: Off");
		break;
	default:
		DebugPrintF("Culling Mode: NA");
		break;
	}

	switch (pipelineState & FillMode) {
	case FillModeSolid:
		DebugPrintF("Fill Mode: Solid");
		break;
	case FillModeWireWireframe:
		DebugPrintF("Fill Mode: Wireframe");
		break;
	default:
		DebugPrintF("Fill Mode: NA");
		break;
	}

	switch (pipelineState & PrimitiveTopology) {
	case PrimitiveTopologyTriangles:
		DebugPrintF("Geometry Primitive: Triangles");
		break;
	case PrimitiveTopologyLines:
		DebugPrintF("Geometry Primitive: Lines");
		break;
	case PrimitiveTopologyPatches:
		DebugPrintF("Geometry Primitive: Patches");
		break;
	case PrimitiveTopologyPoints:
		DebugPrintF("Geometry Primitive: Points");
		break;
	default:
		DebugPrintF("Geometry Primitive: NA");
		break;
	}
}

class __declspec(novtable) IRenderer {
public:
	virtual ~IRenderer() {}
	virtual void Init() = 0;
	virtual void PrepareFrame() = 0;
	virtual void PresentFrame() = 0;
	virtual void EndFrame() = 0;
};

static const std::string DefaultVertexShader = "assets/shaders/default_vert.spv";
static const std::string DefaultFragmentShader = "assets/shaders/default_frag.spv";

struct ShaderId {
private:
	std::string vertexshader;
	std::string fragmentshader;
public:
	uint64_t vertHash = 0;
	uint64_t fragHash = 0;
	ShaderId() : vertexshader(DefaultVertexShader), fragmentshader(DefaultFragmentShader) {
		vertHash = std::hash<std::string>()(vertexshader);
		fragHash = std::hash<std::string>()(fragmentshader);
	}

	ShaderId(std::string vertshader, std::string fragshader) : vertexshader(vertshader), fragmentshader(fragshader) {
		vertHash = std::hash<std::string>()(vertexshader);
		fragHash = std::hash<std::string>()(fragmentshader);
	}

	uint64_t HashCode() const {
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

static const ShaderId DefaultUnlitShader = ShaderId("assets/shaders/default_vert.spv", "assets/shaders/default_frag.spv");
static const ShaderId DefaultLightShader = ShaderId("assets/shaders/phong_vert.spv", "assets/shaders/phong_frag.spv");

struct TextureId {
private:
	std::string texturename;

public:
	uint64_t texturenamehash = 0;
	TextureId() {}

	TextureId(std::string texturefile) :texturename(texturefile) {
		texturenamehash = std::hash<std::string>()(texturename);
	}

	uint64_t HashCode() const {
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

	bool operator!=(const TextureId& other) const {
		return HashCode() != other.HashCode();
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

static const TextureId EmptyTextureId("textures/empty.png");

#endif
