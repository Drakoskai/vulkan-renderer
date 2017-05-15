#pragma once

#include "KaiMath.h"
#include "Vertex.h"
#include "VulkanDrawable.h"
#include "Material.h"

struct SubMesh {
	std::string name;
	std::vector<VertexPTC> verticesPTC;
	/*std::vector<VertexPTN> verticesPTN;
	std::vector<VertexPTNTC> verticesPTNTC;*/
	Vulkan::VulkanRenderer* pRenderer;
	std::vector<uint32_t> indices;
	Vec3 aabbMin;
	Vec3 aabbMax;
	Material material;
	Vulkan::VulkanDrawable* buffer = nullptr;
	SubMesh();
	SubMesh(Vulkan::VulkanRenderer* renderer);
	void SetRenderDevice(Vulkan::VulkanRenderer* renderer) { pRenderer = renderer; }
	void Generate();
	void SolveAABB();
};
