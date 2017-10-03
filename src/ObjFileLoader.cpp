#include "ObjFileLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>
#include "Material.h"
#include "Util.h"

namespace FileFormats {

	void LoadObjFile(const std::string& file, Mesh& mesh) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> mats;
		std::string err;
		std::string filepath(MODELS_PATH + file);

		if (!LoadObj(&attrib, &shapes, &mats, &err, filepath.c_str(), MATERIALS_PATH.c_str())) {
			throw std::runtime_error(err);
		}
		std::vector<Material> materials;
		materials.resize(mats.size());

		for (uint32_t i = 0; i < mats.size(); i++) {
			materials[i].name = mats[i].name;
			materials[i].opacity = mats[i].dissolve;
			materials[i].reflection = mats[i].ior;
			materials[i].ambient = Vec3(mats[i].ambient[0], mats[i].ambient[1], mats[i].ambient[2]);
			materials[i].diffuse = Vec3(mats[i].diffuse[0], mats[i].diffuse[1], mats[i].diffuse[2]);
			materials[i].emissive = Vec3(mats[i].emission[0], mats[i].emission[1], mats[i].emission[2]);
			materials[i].specular = Vec3(mats[i].specular[0], mats[i].specular[1], mats[i].specular[2]);
			materials[i].diffuseTexture = mats[i].diffuse_texname == "" ? EmptyTextureId : mats[i].diffuse_texname;
			/*materials[i].alphaTexture =  mats[i].alpha_texname == "" ? EmptyTextureId :  mats[i].alpha_texname;
			materials[i].ambientTexture =  mats[i].ambient_texname == "" ? EmptyTextureId :  mats[i].ambient_texname;*/
			materials[i].bumpTexture = mats[i].bump_texname == "" ? EmptyTextureId : mats[i].bump_texname;
			/*materials[i].displacementTexture =  mats[i].displacement_texname == "" ? EmptyTextureId :  mats[i].displacement_texname;
			materials[i].specularTexture =  mats[i].specular_texname == "" ? EmptyTextureId :  mats[i].specular_texname;
			materials[i].specularHighlightTexture =  mats[i].specular_highlight_texname == "" ? EmptyTextureId :  mats[i].specular_highlight_texname;*/
			materials[i].shader = DefaultUnlitShader;
			/*if (mats[i].illum == 2) {
				materials[i].shader = DefaultLightShader;
			} else {
				materials[i].shader = DefaultUnlitShader;
			}*/
		}

		std::unordered_map<Vertex, int32_t> uniqueVertices = {};
		for (const auto& shape : shapes) {
			size_t indexOffset = 0;
			for (size_t i = 0; i < shape.mesh.num_face_vertices.size(); i++) {
				auto numfaceVerts = shape.mesh.num_face_vertices[i];
				auto materialId = shape.mesh.material_ids[i];

				for (size_t faceIdx = 0; faceIdx < numfaceVerts; faceIdx++) {
					const auto& index = shape.mesh.indices[indexOffset + faceIdx];
					Vertex vertex = {};
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					vertex.texCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
					if (attrib.normals.size() != 0) {
						vertex.normal = {
							attrib.normals[3 * index.normal_index + 0],
							attrib.normals[3 * index.normal_index + 1],
							attrib.normals[3 * index.normal_index + 2]
						};
					}

					Material material = materials[materialId];
					SubMesh* subMesh = nullptr;
					for (SubMesh& sm : mesh.GetSubMeshes()) {
						if (sm.material == material) {
							subMesh = &sm;
						}
					}
					if (!subMesh) {
						subMesh = mesh.AddSubMesh();
						subMesh->material = material;
					}

					if (uniqueVertices.count(vertex) == 0) {
						uniqueVertices[vertex] = static_cast<int32_t>(subMesh->vertices.size());
						subMesh->vertices.push_back(vertex);
					}
					subMesh->indices.push_back(uniqueVertices[vertex]);
				}
				indexOffset += numfaceVerts;
			}
		}	
	}
}
