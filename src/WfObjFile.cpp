#include "WfObjFile.h"
#include <map>

#include "Mesh.h"


ObjFile::ObjFile(const std::string& path) : m_hasVNormals(false), m_hasTexCoord(false), m_filename(path) { }

ObjFile::~ObjFile() { }

const std::vector<PreMesh>& ObjFile::LoadData() {
	std::fstream fileStream;
	fileStream.open(m_filename.c_str());

	std::string line;
	std::stringstream ss;
	std::string prefix;

	m_meshes.push_back(PreMesh());

	std::vector<Vec3> vertex;
	std::vector<Vec3> vnormal;
	std::vector<Vec2> tcoord;

	while(!fileStream.eof()) {
		getline(fileStream, line);
		ss.clear();
		ss.str(line);
		ss >> prefix;

		if(prefix == "v") {
			float x;
			float y;
			float z;
			ss >> x >> y >> z;
			vertex.push_back(Vec3(x, y, z));
		} else if(prefix == "vn") {
			float xn;
			float yn;
			float zn;
			ss >> xn >> yn >> zn;
			vnormal.push_back(Vec3(xn, yn, zn));
		} else if(prefix == "vt") {
			float u;
			float v;
			ss >> u >> v;
			tcoord.push_back(Vec2(u, 1.0f - v));
		}

		fileStream.close();
		m_hasVNormals = !vnormal.empty();

		fileStream.open(m_filename.c_str());
		fileStream.seekg(std::ios_base::beg);

		while(!fileStream.eof()) {
			getline(fileStream, line);

			if(fileStream.eof()) {
				break;
			}

			ss.clear();
			ss.str(line);
			ss >> prefix;

			if(prefix == "o" || prefix == "g") {
				ReadIndices();
				if(m_meshes.back().name != "unnamed") {
					if(m_meshes.back().face.empty()) {
						m_meshes.erase(begin(m_meshes) + m_meshes.size() - 1);
					}

					m_meshes.push_back(PreMesh());
					m_hasTexCoord = false;
				}

				ss >> m_meshes.back().name;

				m_vertex.clear();
				m_vnormal.clear();
				m_tcoord.clear();

			} else if(prefix == "f") {
				int32_t va;
				int32_t vb;
				int32_t vc;
				int32_t vd;
				int32_t ta;
				unsigned char slash;

				ObjFace face;

				ss >> va;

				if(va < 0) {
					va = static_cast<int32_t>(m_meshes.back().vertex.size()) - va;
				}

				if(m_vertIdxs.find(va - 1) == m_vertIdxs.end()) {
					m_meshes.back().vertex.push_back(vertex[va - 1]);
					m_vertIdxs[va - 1] = static_cast<int32_t>(m_meshes.back().vertex.size()) - 1;
				}
				face.vIdx[0] = static_cast<uint16_t>(va - 1);

				ss >> slash;

				if(slash != '/') {
					ss.unget();

					ss >> vb;
					face.vIdx[1] = static_cast<uint16_t>(vb - 1);

					if(m_vertIdxs.find(vb - 1) == m_vertIdxs.end()) {
						m_meshes.back().vertex.push_back(vertex[vb - 1]);
						m_vertIdxs[vb - 1] = static_cast<int32_t>(m_meshes.back().vertex.size()) - 1;
					}

					ss >> vc;
					face.vIdx[2] = static_cast<uint16_t>(vc - 1);

					if(m_vertIdxs.find(vc - 1) == m_vertIdxs.end()) {
						m_meshes.back().vertex.push_back(vertex[vc - 1]);
						m_vertIdxs[vc - 1] = static_cast<int32_t>(m_meshes.back().vertex.size()) - 1;
					}

					m_meshes.back().face.push_back(face);
					continue;
				}
				ss >> ta;

				if(ta < 0) {
					ta = static_cast<int>(m_meshes.back().uv.size()) - ta;
				}

				if(!ss.fail()) {
					m_hasTexCoord = true;
					if(m_uvIdxs.find(ta - 1) == m_uvIdxs.end()) {
						m_meshes.back().uv.push_back(tcoord[ta - 1]);
						m_uvIdxs[ta - 1] = static_cast<int32_t>(m_meshes.back().uv.size()) - 1;
					}

					face.uvIdx[0] = static_cast<uint16_t>(ta - 1);
				} else {
					ss.clear();
				}

				ss >> slash;

				if(slash == '/') {
					int32_t na;
					ss >> na;

					if(!ss.fail()) {
						if(na < 0) {
							na = static_cast<int32_t>(m_meshes.back().vnormal.size()) - na;
						}

						m_hasVNormals = true;

						if(m_normIdxs.find(na - 1) == m_normIdxs.end()) {
							m_meshes.back().vnormal.push_back(vnormal[na - 1]);
							m_normIdxs[na - 1] = static_cast<int32_t>(m_meshes.back().vnormal.size()) - 1;
						}

						face.vnIdx[0] = static_cast<uint16_t>(na - 1);
					} else {
						ss.clear();
					}
				}

				ss >> vb;
				if(vb < 0) {
					vb = static_cast<int32_t>(m_meshes.back().vertex.size()) - vb;
				}

				if(m_vertIdxs.find(vb - 1) == m_vertIdxs.end()) {
					m_meshes.back().vertex.push_back(vertex[vb - 1]);
					m_vertIdxs[vb - 1] = static_cast<int32_t>(m_meshes.back().vertex.size()) - 1;
				}

				face.vIdx[1] = static_cast<uint16_t>(vb - 1);

				if(m_hasTexCoord) {
					int32_t tb;
					ss >> slash;
					ss >> tb;
					if(tb < 0) {
						tb = static_cast<int32_t>(m_meshes.back().uv.size()) - tb;
					}

					if(m_uvIdxs.find(tb - 1) == m_uvIdxs.end()) {
						m_meshes.back().uv.push_back(tcoord[tb - 1]);
						m_uvIdxs[tb - 1] = static_cast<int32_t>(m_meshes.back().uv.size()) - 1;
					}

					face.uvIdx[1] = static_cast<uint16_t>(tb - 1);
				}

				if(!m_hasTexCoord && m_hasVNormals) {
					ss >> slash;
				}

				if(m_hasVNormals) {
					int32_t nb;
					ss >> slash;
					ss >> nb;
					if(nb < 0) {
						nb = static_cast<int32_t>(m_meshes.back().vnormal.size()) - nb;
					}

					if(m_normIdxs.find(nb - 1) == m_normIdxs.end()) {
						m_meshes.back().vnormal.push_back(vnormal[nb - 1]);
						m_normIdxs[nb - 1] = static_cast<int32_t>(m_meshes.back().vnormal.size()) - 1;
					}

					face.vnIdx[1] = static_cast<uint16_t>(nb - 1);
				}

				ss >> vc;
				if(vc < 0) {
					vc = static_cast<int32_t>(m_meshes.back().vertex.size()) - vc;
				}

				if(m_vertIdxs.find(vc - 1) == m_vertIdxs.end()) {
					m_meshes.back().vertex.push_back(vertex[vc - 1]);
					m_vertIdxs[vc - 1] = static_cast<int32_t>(m_meshes.back().vertex.size()) - 1;
				}

				face.vIdx[2] = static_cast<uint16_t>(vc - 1);

				if(m_hasTexCoord) {
					int32_t tc;
					ss >> slash;
					ss >> tc;
					if(tc < 0) {
						tc = static_cast<int32_t>(m_meshes.back().uv.size()) - tc;
					}

					if(m_uvIdxs.find(tc - 1) == m_uvIdxs.end()) {
						m_meshes.back().uv.push_back(tcoord[tc - 1]);
						m_uvIdxs[tc - 1] = static_cast<int32_t>(m_meshes.back().uv.size()) - 1;
					}

					face.uvIdx[2] = static_cast<uint16_t>(tc - 1);
				}

				if(!m_hasTexCoord && m_hasVNormals) {
					ss >> slash;
				}

				if(m_hasVNormals) {
					int32_t nc;
					ss >> slash;
					ss >> nc;
					if(nc < 0) {
						nc = static_cast<int32_t>(m_meshes.back().vnormal.size()) - nc;
					}

					if(m_normIdxs.find(nc - 1) == m_normIdxs.end()) {
						m_meshes.back().vnormal.push_back(vnormal[nc - 1]);
						m_normIdxs[nc - 1] = static_cast<int32_t>(m_meshes.back().vnormal.size()) - 1;
					}

					face.vnIdx[2] = static_cast<uint16_t>(nc - 1);
				}
				m_meshes.back().face.push_back(face);

				ss >> vd;

				if(!ss.eof()) {
					if(vd < 0) {
						vd = static_cast<int32_t>(m_meshes.back().vertex.size()) - vd;
					}

					if(m_vertIdxs.find(vd - 1) == m_vertIdxs.end()) {
						m_meshes.back().vertex.push_back(vertex[vd - 1]);
						m_vertIdxs[vd - 1] = static_cast<int32_t>(m_meshes.back().vertex.size()) - 1;
					}
					ObjFace face2;
					face2.vIdx[1] = static_cast<uint16_t>(vd - 1);

					face2.vIdx[0] = face.vIdx[2];
					face2.uvIdx[0] = face.uvIdx[2];
					face2.vnIdx[0] = face.vnIdx[2];

					face2.vIdx[2] = face.vIdx[0];
					face2.uvIdx[2] = face.uvIdx[0];
					face2.vnIdx[2] = face.vnIdx[0];

					if(m_hasTexCoord) {
						int32_t td;
						ss >> slash;
						ss >> td;
						if(td < 0) {
							td = static_cast<int32_t>(m_meshes.back().uv.size()) - td;
						}

						if(m_uvIdxs.find(td - 1) == m_uvIdxs.end()) {
							m_meshes.back().uv.push_back(tcoord[td - 1]);
							m_uvIdxs[td - 1] = static_cast<int32_t>(m_meshes.back().uv.size()) - 1;
						}

						face2.uvIdx[1] = static_cast<uint16_t>(td - 1);
					}

					if(!m_hasTexCoord && m_hasVNormals) {
						ss >> slash;
					}

					if(m_hasVNormals) {
						int32_t nd;
						ss >> slash;
						ss >> nd;
						if(nd < 0) {
							nd = static_cast<int32_t>(m_meshes.back().vnormal.size()) - nd;
						}

						if(m_normIdxs.find(nd - 1) == m_normIdxs.end()) {
							m_meshes.back().vnormal.push_back(vnormal[nd - 1]);
							m_normIdxs[nd - 1] = static_cast<int32_t>(m_meshes.back().vnormal.size()) - 1;
						}

						face2.vnIdx[1] = static_cast<uint16_t>(nd - 1);
					}
					m_meshes.back().face.push_back(face2);
				}
			}
		}

		ReadIndices();
	}

	return m_meshes;
}

void ObjFile::ReadIndices() {
	const size_t faces = m_meshes.back().face.size();
	for(size_t f = 0; f < faces; ++f) {
		auto& face = m_meshes.back().face[f];

		face.vIdx[0] = static_cast<uint16_t>(m_vertIdxs[face.vIdx[0]]);
		face.vIdx[1] = static_cast<uint16_t>(m_vertIdxs[face.vIdx[1]]);
		face.vIdx[2] = static_cast<uint16_t>(m_vertIdxs[face.vIdx[2]]);

		if(m_hasVNormals) {
			face.vnIdx[0] = static_cast<uint16_t>(m_normIdxs[face.vnIdx[0]]);
			face.vnIdx[1] = static_cast<uint16_t>(m_normIdxs[face.vnIdx[1]]);
			face.vnIdx[2] = static_cast<uint16_t>(m_normIdxs[face.vnIdx[2]]);
		}

		if(m_hasTexCoord) {
			face.uvIdx[0] = static_cast<uint16_t>(m_uvIdxs[face.uvIdx[0]]);
			face.uvIdx[1] = static_cast<uint16_t>(m_uvIdxs[face.uvIdx[1]]);
			face.uvIdx[2] = static_cast<uint16_t>(m_uvIdxs[face.uvIdx[2]]);
		}
	}
}
