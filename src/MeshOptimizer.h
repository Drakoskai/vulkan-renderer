#ifndef MESH_OPTIMIZER_H__
#define MESH_OPTIMIZER_H__

#include <cstdint>
#include <cmath>

namespace MeshOptimizer {
	inline float ComputeVertexValenceScore(uint32_t numActiveFaces) {
		const float FindVertexScore_ValenceBoostScale = 2.0f;
		const float FindVertexScore_ValenceBoostPower = 0.5f;

		float score = 0;

		// Bonus points for having a low number of tris still to
		// use the vert, so we get rid of lone verts quickly.
		float valenceBoost = pow(static_cast<float>(numActiveFaces), -FindVertexScore_ValenceBoostPower);
		score += FindVertexScore_ValenceBoostScale * valenceBoost;

		return score;
	}
}
#endif
