#include "MeshBuilder.h"
#include "Util.h"

void PreMesh::SolveAABB() {
	const float maxValue = 99999999.0f;
	aabbmin = Vec3(maxValue, maxValue, maxValue);
	aabbmax = Vec3(-maxValue, -maxValue, -maxValue);

	for(size_t v = 0; v < vertex.size(); ++v) {
		const Vec3& pos = vertex[v];

		if(pos.x > maxValue || pos.y > maxValue || pos.z > maxValue) {
			//std::cerr << "AABB's max value is too low in file " << __FILE__ << " near line " << __LINE__ << std::endl;
			//std::cerr << "Vertex " << v << ": " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
		}

		if(pos.x < -maxValue || pos.y < -maxValue || pos.z < -maxValue) {
			//std::cerr << "AABB's min value is too high in file " << __FILE__ << " near line " << __LINE__ << std::endl;
			//std::cerr << "Vertex " << v << ": " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
		}

		aabbmin = glm::min<Vec3>(aabbmin, pos);
		aabbmax = glm::max<Vec3>(aabbmax, pos);
	}
}

void PreMesh::SolveVertexNormals() {
	vnormal.resize(vertex.size());
	for(size_t vertInd = 0; vertInd < vertex.size(); ++vertInd) {
		Vec3 normal;

		for(size_t faceInd = 0; faceInd < face.size(); ++faceInd) {
			face[faceInd].vnIdx[0] = face[faceInd].vIdx[0];
			face[faceInd].vnIdx[1] = face[faceInd].vIdx[1];
			face[faceInd].vnIdx[2] = face[faceInd].vIdx[2];

			const uint16_t& faceA = face[faceInd].vIdx[0];
			const uint16_t& faceB = face[faceInd].vIdx[1];
			const uint16_t& faceC = face[faceInd].vIdx[2];

			const Vec3 va = vertex[faceA];
			Vec3 vb = vertex[faceB] - va;
			const Vec3 vc = vertex[faceC] - va;
			vb = normalize(cross(vb, vc));

			if(face[faceInd].vIdx[0] == vertInd ||
				face[faceInd].vIdx[1] == vertInd ||
				face[faceInd].vIdx[2] == vertInd) {
				normal = normal + vb;
			}
		}

		vnormal[vertInd] = normalize(normal);
	}
}

void PreMesh::Interleave() {
	ObjVertIdx newFace;

	for(size_t f = 0; f < face.size(); ++f) {
		Vec3 tvertex = vertex[face[f].vIdx[0]];
		Vec3 tnormal = vnormal[face[f].vnIdx[0]];
		Vec2 ttcoord = uv.empty() ? Vec2() : uv[face[f].uvIdx[0]];
		Vec4 tcolor = colors.empty() ? Vec4(0, 0, 0, 1) : colors[face[f].cIdx[0]];

		bool found = false;

		for(size_t i = 0; i < m_indices.size(); ++i) {
			if(IsEqual(m_interleavedVerts[m_indices[i].a].position, tvertex) &&
				IsEqual(m_interleavedVerts[m_indices[i].a].normal, tnormal) &&
				IsEqual(m_interleavedVerts[m_indices[i].a].texCoord, ttcoord) &&
				IsEqual(m_interleavedVerts[m_indices[i].a].color, tcolor)) {
				found = true;
				newFace.a = m_indices[i].a;
				break;
			}
		}

		if(!found) {
			VertexPTNTC newVertex;
			newVertex.position = tvertex;
			newVertex.normal = tnormal;
			newVertex.texCoord = ttcoord;
			newVertex.color = tcolor;

			m_interleavedVerts.push_back(newVertex);

			newFace.a = static_cast<uint16_t>(m_interleavedVerts.size() - 1);
		}

		tvertex = vertex[face[f].vIdx[1]];
		tnormal = vnormal[face[f].vnIdx[1]];
		ttcoord = uv.empty() ? Vec2() : uv[face[f].uvIdx[1]];
		tcolor = colors.empty() ? Vec4(0, 0, 0, 1) : colors[face[f].cIdx[1]];

		found = false;

		for(size_t i = 0; i < m_indices.size(); ++i) {
			if(IsEqual(m_interleavedVerts[m_indices[i].b].position, tvertex) &&
				IsEqual(m_interleavedVerts[m_indices[i].b].normal, tnormal) &&
				IsEqual(m_interleavedVerts[m_indices[i].b].texCoord, ttcoord) &&
				IsEqual(m_interleavedVerts[m_indices[i].b].color, tcolor))

			{
				found = true;

				newFace.b = m_indices[i].b;
				break;
			}
		}

		if(!found) {
			VertexPTNTC newVertex;
			newVertex.position = tvertex;
			newVertex.normal = tnormal;
			newVertex.texCoord = ttcoord;
			newVertex.color = tcolor;

			m_interleavedVerts.push_back(newVertex);

			newFace.b = static_cast<uint16_t>(m_interleavedVerts.size() - 1);
		}

		tvertex = vertex[face[f].vIdx[2]];
		tnormal = vnormal[face[f].vnIdx[2]];
		ttcoord = uv.empty() ? Vec2() : uv[face[f].uvIdx[2]];
		tcolor = colors.empty() ? Vec4(0, 0, 0, 1) : colors[face[f].cIdx[2]];

		found = false;

		for(size_t i = 0; i < m_indices.size(); ++i) {
			if(IsEqual(m_interleavedVerts[m_indices[i].c].position, tvertex) &&
				IsEqual(m_interleavedVerts[m_indices[i].c].normal, tnormal) &&
				IsEqual(m_interleavedVerts[m_indices[i].c].texCoord, ttcoord) &&
				IsEqual(m_interleavedVerts[m_indices[i].c].color, tcolor)) {
				found = true;

				newFace.c = m_indices[i].c;
				break;
			}
		}

		if(!found) {
			VertexPTNTC newVertex;
			newVertex.position = tvertex;
			newVertex.normal = tnormal;
			newVertex.texCoord = ttcoord;
			newVertex.color = tcolor;

			m_interleavedVerts.push_back(newVertex);

			newFace.c = static_cast<uint16_t>(m_interleavedVerts.size() - 1);
		}

		m_indices.push_back(newFace);
	}

	if(m_interleavedVerts.size() > 65536) {
		throw std::runtime_error("has more than 65536 vertices!");
	}
}

void PreMesh::OptimizeFaces() {
	m_verticesWithCachedata.resize(m_interleavedVerts.size());

	for(size_t i = 0; i < m_interleavedVerts.size(); ++i) {
		m_verticesWithCachedata[i].color = m_interleavedVerts[i].color;
		m_verticesWithCachedata[i].texCoord = m_interleavedVerts[i].texCoord;
		m_verticesWithCachedata[i].position = m_interleavedVerts[i].position;
		m_verticesWithCachedata[i].tangent = m_interleavedVerts[i].tangent;
		m_verticesWithCachedata[i].normal = m_interleavedVerts[i].normal;
	}

	// Face count per vertex
	for(size_t i = 0; i < m_indices.size(); ++i) {
		++m_verticesWithCachedata[m_indices[i].a].data.activeFaceListSize;
		++m_verticesWithCachedata[m_indices[i].b].data.activeFaceListSize;
		++m_verticesWithCachedata[m_indices[i].c].data.activeFaceListSize;
	}

	std::vector<uint32_t> activeFaceList;

	const uint16_t kEvictedCacheIndex = std::numeric_limits<uint16_t>::max();

	{
		// allocate face list per vertex
		uint32_t curActiveFaceListPos = 0;

		for(size_t i = 0; i < m_verticesWithCachedata.size(); ++i) {
			VertexPTNTCWithData& vertexData = m_verticesWithCachedata[i];
			vertexData.data.cachePos0 = kEvictedCacheIndex;
			vertexData.data.cachePos1 = kEvictedCacheIndex;
			vertexData.data.activeFaceListStart = curActiveFaceListPos;
			curActiveFaceListPos += vertexData.data.activeFaceListSize;
			vertexData.data.score = FindVertexScore(vertexData.data.activeFaceListSize, vertexData.data.cachePos0, lruCacheSize);
			vertexData.data.activeFaceListSize = 0;
		}
		activeFaceList.resize(curActiveFaceListPos);
	}

	for(uint32_t i = 0; i < static_cast<uint32_t>(m_indices.size()); ++i) {
		uint16_t index = m_indices[i].a;
		VertexPTNTCWithData& vertexDataA = m_verticesWithCachedata[index];
		activeFaceList[vertexDataA.data.activeFaceListStart + vertexDataA.data.activeFaceListSize] = i;
		++vertexDataA.data.activeFaceListSize;

		index = m_indices[i].b;
		VertexPTNTCWithData& vertexDataB = m_verticesWithCachedata[index];
		activeFaceList[vertexDataB.data.activeFaceListStart + vertexDataB.data.activeFaceListSize] = i;
		++vertexDataB.data.activeFaceListSize;

		index = m_indices[i].c;
		VertexPTNTCWithData& vertexDataC = m_verticesWithCachedata[index];
		activeFaceList[vertexDataC.data.activeFaceListStart + vertexDataC.data.activeFaceListSize] = i;
		++vertexDataC.data.activeFaceListSize;
	}

	std::vector<uint8_t> processedFaceList;
	processedFaceList.resize(m_indices.size());

	uint16_t vertexCacheBuffer[(MaxVertexCacheSize + 3) * 2];
	uint16_t* cache0 = vertexCacheBuffer;
	uint16_t* cache1 = vertexCacheBuffer + (MaxVertexCacheSize + 3);
	uint16_t entriesInCache0 = 0;

	uint32_t bestFace = 0;
	float bestScore = -1.f;

	const float maxValenceScore = FindVertexScore(1, kEvictedCacheIndex, lruCacheSize) * 3.0f;

	m_newIndexList.resize(m_indices.size());

	for(uint32_t i = 0; i < m_indices.size(); ++i) {
		if(bestScore < 0.f) {
			for(uint32_t j = 0; j < m_indices.size(); ++j) {
				if(processedFaceList[j] == 0) {
					uint32_t fface = j;
					float faceScore = 0.f;

					uint16_t indexA = m_indices[fface].a;
					VertexPTNTCWithData& vertexDataA = m_verticesWithCachedata[indexA];
					assert(vertexDataA.data.activeFaceListSize > 0);
					assert(vertexDataA.data.cachePos0 >= lruCacheSize);
					faceScore += vertexDataA.data.score;

					uint16_t indexB = m_indices[fface].b;
					VertexPTNTCWithData& vertexDataB = m_verticesWithCachedata[indexB];
					assert(vertexDataB.data.activeFaceListSize > 0);
					assert(vertexDataB.data.cachePos0 >= lruCacheSize);
					faceScore += vertexDataB.data.score;

					uint16_t indexC = m_indices[fface].c;
					VertexPTNTCWithData& vertexDataC = m_verticesWithCachedata[indexC];
					assert(vertexDataC.data.activeFaceListSize > 0);
					assert(vertexDataC.data.cachePos0 >= lruCacheSize);
					faceScore += vertexDataC.data.score;

					if(faceScore > bestScore) {
						bestScore = faceScore;
						bestFace = fface;

						assert(bestScore <= maxValenceScore);
						if(bestScore >= maxValenceScore) {
							break;
						}
					}
				}
			}
			assert(bestScore >= 0.f);
		}

		processedFaceList[bestFace] = 1;
		uint16_t entriesInCache1 = 0;

		{
			uint16_t indexA = m_indices[bestFace].a;
			m_newIndexList[i].a = indexA;

			VertexPTNTCWithData& vertexData = m_verticesWithCachedata[indexA];
			bool skip = false;

			if(vertexData.data.cachePos1 >= entriesInCache1) {
				vertexData.data.cachePos1 = entriesInCache1;
				cache1[entriesInCache1++] = indexA;

				if(vertexData.data.activeFaceListSize == 1) {
					--vertexData.data.activeFaceListSize;
					skip = true;
				}
			}

			if(!skip) {
				assert(vertexData.data.activeFaceListSize > 0);
				uint32_t* begin = &activeFaceList[vertexData.data.activeFaceListStart];
				uint32_t* end = &activeFaceList[vertexData.data.activeFaceListStart + vertexData.data.activeFaceListSize];
				uint32_t* it = std::find(begin, end, bestFace);
				assert(it != end);
				std::swap(*it, *(end - 1));
				--vertexData.data.activeFaceListSize;
				vertexData.data.score = FindVertexScore(vertexData.data.activeFaceListSize, vertexData.data.cachePos1, lruCacheSize);
			}
		}

		{
			uint16_t indexB = m_indices[bestFace].b;
			m_newIndexList[i].b = indexB;

			VertexPTNTCWithData& vertexData = m_verticesWithCachedata[indexB];
			bool skip = false;

			if(vertexData.data.cachePos1 >= entriesInCache1) {
				vertexData.data.cachePos1 = entriesInCache1;
				cache1[entriesInCache1++] = indexB;

				if(vertexData.data.activeFaceListSize == 1) {
					--vertexData.data.activeFaceListSize;
					skip = true;
				}
			}

			if(!skip) {
				assert(vertexData.data.activeFaceListSize > 0);
				uint32_t* begin = &activeFaceList[vertexData.data.activeFaceListStart];
				uint32_t* end = &activeFaceList[vertexData.data.activeFaceListStart + vertexData.data.activeFaceListSize];
				auto it = std::find(begin, end, bestFace);
				assert(it != end);
				std::swap(*it, *(end - 1));
				--vertexData.data.activeFaceListSize;
				vertexData.data.score = FindVertexScore(vertexData.data.activeFaceListSize, vertexData.data.cachePos1, lruCacheSize);
			}
		}

		{
			uint16_t indexC = m_indices[bestFace].c;
			m_newIndexList[i].c = indexC;

			VertexPTNTCWithData& vertexData = m_verticesWithCachedata[indexC];
			bool skip = false;

			if(vertexData.data.cachePos1 >= entriesInCache1) {
				vertexData.data.cachePos1 = entriesInCache1;
				cache1[entriesInCache1++] = indexC;

				if(vertexData.data.activeFaceListSize == 1) {
					--vertexData.data.activeFaceListSize;
					skip = true;
				}
			}

			if(!skip) {
				assert(vertexData.data.activeFaceListSize > 0);
				uint32_t* begin = &activeFaceList[vertexData.data.activeFaceListStart];
				uint32_t* end = &activeFaceList[vertexData.data.activeFaceListStart + vertexData.data.activeFaceListSize];
				uint32_t* it = std::find(begin, end, bestFace);
				assert(it != end);
				std::swap(*it, *(end - 1));
				--vertexData.data.activeFaceListSize;
				vertexData.data.score = FindVertexScore(vertexData.data.activeFaceListSize, vertexData.data.cachePos1, lruCacheSize);
			}
		}

		for(uint32_t c0 = 0; c0 < entriesInCache0; ++c0) {
			uint16_t index = cache0[c0];
			VertexPTNTCWithData& vertexData = m_verticesWithCachedata[index];

			if(vertexData.data.cachePos1 >= entriesInCache1) {
				vertexData.data.cachePos1 = entriesInCache1;
				cache1[entriesInCache1++] = index;
				vertexData.data.score = FindVertexScore(vertexData.data.activeFaceListSize, vertexData.data.cachePos1, lruCacheSize);
			}
		}

		bestScore = -1.f;
		for(uint32_t c1 = 0; c1 < entriesInCache1; ++c1) {
			uint16_t index = cache1[c1];
			VertexPTNTCWithData& vertexData = m_verticesWithCachedata[index];
			vertexData.data.cachePos0 = vertexData.data.cachePos1;
			vertexData.data.cachePos1 = kEvictedCacheIndex;
			for(uint32_t j = 0; j < vertexData.data.activeFaceListSize; ++j) {
				uint32_t fface = activeFaceList[vertexData.data.activeFaceListStart + j];
				float faceScore = 0.f;

				uint16_t faceIndexA = m_indices[fface].a;
				VertexPTNTCWithData& faceVertexDataA = m_verticesWithCachedata[faceIndexA];
				faceScore += faceVertexDataA.data.score;

				uint16_t faceIndexB = m_indices[fface].b;
				VertexPTNTCWithData& faceVertexDataB = m_verticesWithCachedata[faceIndexB];
				faceScore += faceVertexDataB.data.score;

				uint16_t faceIndexC = m_indices[fface].c;
				VertexPTNTCWithData& faceVertexDataC = m_verticesWithCachedata[faceIndexC];
				faceScore += faceVertexDataC.data.score;

				if(faceScore > bestScore) {
					bestScore = faceScore;
					bestFace = fface;
				}
			}
		}

		std::swap(cache0, cache1);
		entriesInCache0 = Min(entriesInCache1, lruCacheSize);
	}

	m_indices = m_newIndexList;
}

float PreMesh::FindVertexScore(uint32_t numActiveFaces, uint32_t cachePosition, uint32_t vertexCacheSize) {
	if(numActiveFaces == 0) {
		return -1.0f;
	}

	float score = 0.f;
	if(cachePosition < vertexCacheSize) {
		score += m_vertexCacheScores[vertexCacheSize][cachePosition];
	}

	if(numActiveFaces < MaxPrecomputedVertexValenceScores) {
		score += m_vertexValenceScores[numActiveFaces];
	} else {
		score += ComputeVertexValenceScore(numActiveFaces);
	}

	return score;
}

float PreMesh::ComputeVertexValenceScore(uint32_t numActiveFaces) const {
	const float FindVertexScore_ValenceBoostScale = 2.0f;
	const float FindVertexScore_ValenceBoostPower = 0.5f;

	float score = 0;
	float valenceBoost = pow(static_cast<float>(numActiveFaces), -FindVertexScore_ValenceBoostPower);
	score += FindVertexScore_ValenceBoostScale * valenceBoost;

	return score;
}


void PreMesh::SolveFaceNormals() {
	fnormal.resize(m_indices.size());

	for(size_t faceInd = 0; faceInd < m_indices.size(); ++faceInd) {
		const uint16_t& faceA = m_indices[faceInd].a;
		const uint16_t& faceB = m_indices[faceInd].b;
		const uint16_t& faceC = m_indices[faceInd].c;

		const Vec3 va = m_interleavedVerts[faceA].position;
		Vec3 vb = m_interleavedVerts[faceB].position;
		Vec3 vc = m_interleavedVerts[faceC].position;

		vb -= va;
		vc -= va;

		vb = normalize(cross(vb, vc));

		fnormal[faceInd] = vb;
	}
}

void PreMesh::SolveFaceTangents() {
	assert(!m_indices.empty());

	tangents.resize(m_indices.size());
	bitangents.resize(m_indices.size());

	bool degenerateFound = false;

	// Algorithm source:
	// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/#header-3
	for(size_t f = 0; f < m_indices.size(); ++f) {
		const Vec3& p1 = m_interleavedVerts[m_indices[f].a].position;
		const Vec3& p2 = m_interleavedVerts[m_indices[f].b].position;
		const Vec3& p3 = m_interleavedVerts[m_indices[f].c].position;

		const Vec2& uv1 = m_interleavedVerts[m_indices[f].a].texCoord;
		const Vec2& uv2 = m_interleavedVerts[m_indices[f].b].texCoord;
		const Vec2& uv3 = m_interleavedVerts[m_indices[f].c].texCoord;

		const Vec2 deltaUV1 = uv2 - uv1;
		const Vec2 deltaUV2 = uv3 - uv1;

		const float area = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;

		Vec3 tangent;
		Vec3 bitangent;

		if(fabs(area) > 0.00001f) {
			const Vec3 deltaP1 = p2 - p1;
			const Vec3 deltaP2 = p3 - p1;

			tangent = (deltaP1 * deltaUV2.y - deltaP2 * deltaUV1.y) / area;
			bitangent = (deltaP2 * deltaUV1.x - deltaP1 * deltaUV2.x) / area;
		} else {
			degenerateFound = true;
		}

		const Vec4 tangent4(tangent.x, tangent.y, tangent.z, 0.0f);
		tangents[f] = tangent4;
		bitangents[f] = bitangent;
	}

	if(degenerateFound) {
		DebugPrintF("Warning: Degenerate UV map in mesh %s. Author needs to separate texture points.", name);
	}
}

void PreMesh::SolveVertexTangents() {
	assert(!tangents.empty());

	for(size_t v = 0; v < m_interleavedVerts.size(); ++v) {
		m_interleavedVerts[v].tangent = Vec4(0, 0, 0, 0);
	}

	vbitangents.resize(m_interleavedVerts.size());

	// http://www.terathon.com/code/tangent.html
	// "To find the tangent vectors for a single vertex, we average the tangents for all triangles sharing that vertex"
	for(size_t vertInd = 0; vertInd < m_interleavedVerts.size(); ++vertInd) {
		Vec4 tangent(0.0f, 0.0f, 0.0f, 0.0f);
		Vec3 bitangent;

		for(size_t faceInd = 0; faceInd < m_indices.size(); ++faceInd) {
			if(m_indices[faceInd].a == vertInd ||
				m_indices[faceInd].b == vertInd ||
				m_indices[faceInd].c == vertInd) {
				tangent += tangents[faceInd];
				bitangent += bitangents[faceInd];
			}
		}

		m_interleavedVerts[vertInd].tangent = tangent;
		vbitangents[vertInd] = bitangent;
	}

	for(size_t v = 0; v < m_interleavedVerts.size(); ++v) {
		Vec4& tangent = m_interleavedVerts[v].tangent;
		const Vec3& normal = m_interleavedVerts[v].normal;
		const Vec4 normal4(normal.x, normal.y, normal.z, 0);

		// Gram-Schmidt orthonormalization.
		tangent -= normal4 * dot(normal4, tangent);
		normalize(tangent);

		// Handedness. TBN must form a right-handed coordinate system,
		// i.e. cross(n,t) must have the same orientation as b.
		const Vec3 cp = cross(normal, Vec3(tangent.x, tangent.y, tangent.z));
		tangent.w = dot(cp, vbitangents[v]) >= 0 ? 1.0f : -1.0f;
	}
}

void PreMesh::CopyInterleavedVerticesToPTN() {
	m_interleavedVertsPTN.resize(m_interleavedVerts.size());

	for(size_t i = 0; i < m_interleavedVerts.size(); ++i) {
		m_interleavedVertsPTN[i].position = m_interleavedVerts[i].position;
		m_interleavedVertsPTN[i].texCoord = m_interleavedVerts[i].texCoord;
		m_interleavedVertsPTN[i].normal = m_interleavedVerts[i].normal;
	}
}