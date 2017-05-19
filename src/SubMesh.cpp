#include "SubMesh.h"
#include "VulkanRenderer.h"

SubMesh::SubMesh() : pRenderer(nullptr) {}

SubMesh::SubMesh(Vulkan::VulkanRenderer* renderer) : pRenderer(renderer), aabbMin(), aabbMax() {}

void SubMesh::Generate() {
	if (!pRenderer) { return; }
	SolveAABB();
	buffer = pRenderer->GetDrawable();
	buffer->Generate(vertices, indices, material);
}

void SubMesh::SolveAABB() {
	if (!pRenderer) { return; }

	const float MaxFloatValue = 99999999.0f;
	aabbMin = Vec3(MaxFloatValue, MaxFloatValue, MaxFloatValue);
	aabbMax = Vec3(-MaxFloatValue, -MaxFloatValue, -MaxFloatValue);

	for (size_t v = 0; v < vertices.size(); ++v) {
		const Vec3& pos = vertices[v].position;
		aabbMin = Min(aabbMin, pos);
		aabbMax = Max(aabbMax, pos);
	}
}