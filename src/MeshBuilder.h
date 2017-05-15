#pragma once

#include "pch.h"
#include "Vertex.h"

struct ObjVertIdx {
	uint16_t a;
	uint16_t b;
	uint16_t c;
};

struct ObjFace {
	uint16_t vIdx[3];
	uint16_t uvIdx[3];
	uint16_t vnIdx[3];
	uint16_t cIdx[3];

	ObjFace() {
		vIdx[0] = vIdx[1] = vIdx[2] = 0;
		uvIdx[0] = uvIdx[1] = uvIdx[2] = 0;
		vnIdx[0] = vnIdx[1] = vnIdx[2] = 0;
		cIdx[0] = cIdx[1] = cIdx[2] = 0;
	}
};

struct VertexData {
	float score = 0;
	uint32_t activeFaceListStart = 0;
	uint32_t activeFaceListSize = 0;
	uint32_t cachePos0 = 0;
	uint32_t cachePos1 = 0;
};

struct VertexPTNTCWithData {
	Vec3 position;
	Vec2 texCoord;
	Vec3 normal;
	Vec4 tangent;
	Vec4 color;
	VertexData data;
};

struct PreMesh {
	static const uint32_t MaxVertexCacheSize = 64;
	static const uint32_t MaxPrecomputedVertexValenceScores = 64;
	static const uint16_t lruCacheSize = 64;

	std::vector<Vec3> vertex;
	std::vector<Vec4> tangents;
	std::vector<Vec3> bitangents;
	std::vector<Vec3> vbitangents;
	std::vector<Vec3> vnormal;
	std::vector<Vec4> colors;
	std::vector<Vec3> fnormal;
	std::vector<Vec2> uv;
	std::vector<ObjFace> face;

	std::vector<VertexPTNTC> m_interleavedVerts;
	std::vector<VertexPTN> m_interleavedVertsPTN;
	std::vector<ObjVertIdx> m_indices;

	std::vector<VertexPTNTCWithData> m_verticesWithCachedata;

	std::string name;

	float m_vertexCacheScores[MaxVertexCacheSize + 1][MaxVertexCacheSize];
	float m_vertexValenceScores[MaxPrecomputedVertexValenceScores];

	std::vector<ObjVertIdx> m_newIndexList;

	Vec3 aabbmin;
	Vec3 aabbmax;

	void SolveAABB();
	void SolveVertexNormals();
	void Interleave();
	void OptimizeFaces();
	float FindVertexScore(uint32_t numActiveFaces, uint32_t cachePosition, uint32_t vertexCacheSize);
	float ComputeVertexValenceScore(uint32_t numActiveFaces) const;
	void SolveFaceNormals();
	void SolveFaceTangents();
	void SolveVertexTangents();
	void CopyInterleavedVerticesToPTN();

};
