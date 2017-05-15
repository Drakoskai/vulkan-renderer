#include "ObjFileLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>
#include "Util.h"
#include "Material.h"

namespace FileFormats {

	void LoadObjFile(const std::string& file, Mesh& mesh) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;

		if(!LoadObj(&attrib, &shapes, &materials, &err, file.c_str())) {
			throw std::runtime_error(err);
		}

		for(const auto& material : materials) {
			DebugPrintF("Material: %s\n", material.name.c_str());
		}

		for(const auto& shape : shapes) {
			SubMesh* subMesh = mesh.AddSubMesh();
			std::vector<VertexPTC> vertices;
			std::vector<uint32_t> indices;

			std::unordered_map<VertexPTC, int32_t> uniqueVertices = { };
			subMesh->name = shape.name;

			for(const auto& index : shape.mesh.indices) {
				VertexPTC vertex = { };
				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
				vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

				if(uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}

			subMesh->verticesPTC = vertices;
			subMesh->indices = indices;
		}
	}

	void LoadObjFile(const std::string& file, std::vector<VertexPTC>& vertices, std::vector<uint32_t>& indices, std::vector<Material>& materials) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> mats;
		std::string err;

		if(!LoadObj(&attrib, &shapes, &mats, &err, file.c_str(), "models/")) {
			throw std::runtime_error(err);
		}
		
		for(const auto& mat : mats) {
			Material material = { };
			material.name = mat.name;
			material.ambient = Vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
			material.diffuse = Vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
			material.emissive = Vec3(mat.emission[0], mat.emission[1], mat.emission[2]);
			material.specular = Vec3(mat.specular[0], mat.specular[1], mat.specular[2]);
			material.diffuseTexture = mat.diffuse_texname;
			materials.push_back(material);
		}
		
		for(const auto& shape : shapes) {
			std::unordered_map<VertexPTC, int> uniqueVertices = { };
			for(const auto& index : shape.mesh.indices) {
				VertexPTC vertex = { };
				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
				vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

				if(uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}
}