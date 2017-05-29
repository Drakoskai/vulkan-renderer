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
		std::vector<MaterialGroup> materialGroups;

		for (size_t i = 0; i < materials.size(); i++) {
			auto mat = materials[i];

			MaterialGroup group;
			Material material = {};
			material.name = mat.name;
			material.opacity = mat.dissolve;
			material.reflection = mat.ior;
			material.ambient = Vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
			material.diffuse = Vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
			material.emissive = Vec3(mat.emission[0], mat.emission[1], mat.emission[2]);
			material.specular = Vec3(mat.specular[0], mat.specular[1], mat.specular[2]);
			material.diffuseTexture = mat.diffuse_texname;
			material.alphaTexture = mat.alpha_texname;
			material.ambientTexture = mat.ambient_texname;
			material.bumpTexture = mat.bump_texname;
			material.displacementTexture = mat.displacement_texname;
			material.specularTexture = mat.specular_texname;
			material.specularHighlightTexture = mat.specular_highlight_texname;

			if (mat.illum == 2) {
				material.shader = DefaultLightShader;
			} else {
				material.shader = DefaultUnlitShader;
			}

			group.material = material;
			materialGroups.push_back(group);
		}

		std::unordered_map<Vertex, int32_t> uniqueVertices = {};
		auto appendVertex = [&uniqueVertices, &materialGroups](const Vertex& vertex, int material_id) {
			auto& uniqueVerticesRef = uniqueVertices;
			auto& group = materialGroups[material_id];
			if (uniqueVerticesRef.count(vertex) == 0) {
				uniqueVerticesRef[vertex] = static_cast<int32_t>(group.vertices.size());
				group.vertices.push_back(vertex);
			}
			group.indices.push_back(uniqueVerticesRef[vertex]);
		};
		SubMesh* subMesh = mesh.AddSubMesh();
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
					appendVertex(vertex, materialId);
				}
				indexOffset += numfaceVerts;
			}			
		}
		for (auto matgroup : materialGroups) {
			if (matgroup.indices.size() != 0) {
				subMesh->groups.push_back(matgroup);
			}
		}
	}
}
