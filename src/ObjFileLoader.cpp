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
		std::vector<tinyobj::material_t> materials;
		std::string err;
		std::string filepath(MODELS_PATH + file);

		if (!LoadObj(&attrib, &shapes, &materials, &err, filepath.c_str(), MATERIALS_PATH.c_str())) {
			throw std::runtime_error(err);
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

					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};

					auto& mat = materials[materialId];
					Material material = {};
					material.name = mat.name;
					material.opacity = mat.dissolve;
					material.reflection = mat.ior;
					material.ambient = Vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
					material.diffuse = Vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
					material.emissive = Vec3(mat.emission[0], mat.emission[1], mat.emission[2]);
					material.specular = Vec3(mat.specular[0], mat.specular[1], mat.specular[2]);
					material.diffuseTexture = mat.diffuse_texname == "" ? EmptyTextureId : mat.diffuse_texname;
					material.alphaTexture = mat.alpha_texname == "" ? EmptyTextureId : mat.alpha_texname;
					material.ambientTexture = mat.ambient_texname == "" ? EmptyTextureId : mat.ambient_texname;
					material.bumpTexture = mat.bump_texname == "" ? EmptyTextureId : mat.bump_texname;
					material.displacementTexture = mat.displacement_texname == "" ? EmptyTextureId : mat.displacement_texname;
					material.specularTexture = mat.specular_texname == "" ? EmptyTextureId : mat.specular_texname;
					material.specularHighlightTexture = mat.specular_highlight_texname == "" ? EmptyTextureId : mat.specular_highlight_texname;

					if (mat.illum == 2) {
						if(material.bumpTexture != EmptyTextureId)
						{
							material.shader = ShaderId("assets/shaders/phong_vert.spv", "assets/shaders/phong_dn_frag.spv");
						}else
						{
							material.shader = DefaultLightShader;
						}						
					} else {
						material.shader = DefaultUnlitShader;
					}

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
