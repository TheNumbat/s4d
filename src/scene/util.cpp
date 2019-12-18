
#include "util.h"

#include <misc/tiny_obj_loader.h>

namespace Util {

	std::string obj_mesh(std::string obj_file, GL::Mesh& mesh) {

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string warn, err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_file.c_str());

		if(!ret) {
			return "Failed to load mesh '" + last_file(obj_file) + "': unknown error";
		}
		if (!err.empty()) {
			return "Failed to load mesh '" + last_file(obj_file) + "': " + err;
		}
		if(shapes.size() == 0) {
			return "Failed to load mesh '" + last_file(obj_file) + "': no shapes found.";
		}

		std::vector<GL::Mesh::Vert> verts;

		for (size_t s = 0; s < shapes.size(); s++) {
			
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

				int fv = shapes[s].mesh.num_face_vertices[f];

				for (size_t v = 0; v < fv; v++) {
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

					if(idx.normal_index == -1) {
						return "Failed to load mesh '" + last_file(obj_file) + "': no vertex normals.";
					}

					float vx = attrib.vertices[3*idx.vertex_index+0];
					float vy = attrib.vertices[3*idx.vertex_index+1];
					float vz = attrib.vertices[3*idx.vertex_index+2];
					float nx = attrib.normals[3*idx.normal_index+0];
					float ny = attrib.normals[3*idx.normal_index+1];
					float nz = attrib.normals[3*idx.normal_index+2];
					verts.push_back({{vx, vy, vz}, {nx, ny, nz}});
				}
				index_offset += fv;
			}
		}

		mesh = GL::Mesh(verts);
		return {};
	}

	GL::Mesh cube_mesh(float r) {

		static const std::vector<GL::Mesh::Vert> verts = {
			{{-r,-r,-r},{-1.0f,0.0f,0.0f}},
			{{-r,-r,r},{-1.0f,0.0f,0.0f}},
			{{-r,r,r},{-1.0f,0.0f,0.0f}}, // Left Side
			{{-r,-r,-r},{-1.0f,0.0f,0.0f}},
			{{-r,r,r},{-1.0f,0.0f,0.0f}},
			{{-r,r,-r},{-1.0f,0.0f,0.0f}}, // Left Side
			{{r,r,-r},{0.0f,0.0f,-1.0f}},
			{{-r,-r,-r},{0.0f,0.0f,-1.0f}},
			{{-r,r,-r},{0.0f,0.0f,-1.0f}}, // Back Side
			{{r,-r,r},{0.0f,-1.0f,0.0f}},
			{{-r,-r,-r},{0.0f,-1.0f,0.0f}},
			{{r,-r,-r},{0.0f,-1.0f,0.0f}}, // Bottom Side
			{{r,r,-r},{0.0f,0.0f,-1.0f}},
			{{r,-r,-r},{0.0f,0.0f,-1.0f}},
			{{-r,-r,-r},{0.0f,0.0f,-1.0f}}, // Back Side
			{{r,-r,r},{0.0f,-1.0f,0.0f}},
			{{-r,-r,r},{0.0f,-1.0f,0.0f}},
			{{-r,-r,-r},{0.0f,-1.0f,0.0f}}, // Bottom Side
			{{-r,r,r},{0.0f,0.0f,1.0f}},
			{{-r,-r,r},{0.0f,0.0f,1.0f}},
			{{r,-r,r},{0.0f,0.0f,1.0f}}, // Front Side
			{{r,r,r},{1.0f,0.0f,0.0f}},
			{{r,-r,-r},{1.0f,0.0f,0.0f}},
			{{r,r,-r},{1.0f,0.0f,0.0f}}, // Right Side
			{{r,-r,-r},{1.0f,0.0f,0.0f}},
			{{r,r,r},{1.0f,0.0f,0.0f}},
			{{r,-r,r},{1.0f,0.0f,0.0f}}, // Right Side
			{{r,r,r},{0.0f,1.0f,0.0f}},
			{{r,r,-r},{0.0f,1.0f,0.0f}},
			{{-r,r,-r},{0.0f,1.0f,0.0f}}, // Top Side
			{{r,r,r},{0.0f,1.0f,0.0f}},
			{{-r,r,-r},{0.0f,1.0f,0.0f}},
			{{-r,r,r},{0.0f,1.0f,0.0f}}, // Top Side
			{{r,r,r},{0.0f,0.0f,1.0f}},
			{{-r,r,r},{0.0f,0.0f,1.0f}},
			{{r,-r,r},{0.0f,0.0f,1.0f}}   // Front Side
		};

		return GL::Mesh(verts);
	}
}
