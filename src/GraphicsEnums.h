#pragma once

#include <cstdint>

enum class ClearFlags : uint32_t { Color = 1 << 0, Depth = 1 << 1, DontClear = 1 << 2 };

enum class BlendMode { AlphaBlend, Additive, Off };

enum class DepthFunction { LessOrEqualWriteOff, LessOrEqualWriteOn, NoneWriteOff };

enum class TextureWrap { Repeat, Clamp };

enum class TextureFilter { Nearest, Linear };

enum class Mipmaps { Generate, None };

enum class ColorSpace { RGB, SRGB };

enum class Anisotropy { k1, k2, k4, k8 };

enum class CullMode { Back, Front, Off };

enum class FillMode { Solid, Wireframe };

enum class PrimitiveTopology { Triangles, Lines, Patches, Points };