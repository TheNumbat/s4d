
#include "util.h"

#include <misc/tiny_obj_loader.h>

namespace Util {

	GL::Mesh cyl_mesh(float radius, float height) {
		return cone_mesh(radius, radius, height);
	}

	GL::Mesh arrow_mesh() {
		auto base = Detail::cone_verts(0.03f, 0.03f, 0.7f);
		auto tip = Detail::cone_verts(0.075f, 0.005f, 0.2f);
		for(auto& v : tip) v.pos.y += 0.7f;
		base.insert(base.end(), tip.begin(), tip.end());
		return GL::Mesh(base);
	}

	GL::Mesh scale_mesh() {
		auto base = Detail::cone_verts(0.03f, 0.03f, 0.7f);
		auto tip = Detail::cube_verts(0.1f);
		for(auto& v : tip) v.pos.y += 0.7f;
		base.insert(base.end(), tip.begin(), tip.end());
		return GL::Mesh(base);
	}
	
	GL::Mesh cone_mesh(float bradius, float tradius, float height) {
		return GL::Mesh(Detail::cone_verts(bradius, tradius, height));
	}

	GL::Mesh torus_mesh(float iradius, float oradius) {
		return GL::Mesh(Detail::torus_verts(iradius, oradius));
	}

	GL::Mesh cube_mesh(float r) {
		return GL::Mesh(Detail::cube_verts(r));
	}

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

	namespace Detail {

		std::vector<GL::Mesh::Vert> cube_verts(float r) {
			return {
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
		}

		// https://wiki.unity3d.com/index.php/ProceduralPrimitives
		std::vector<GL::Mesh::Vert> cone_verts(float bradius, float tradius, float height) {

			const int n_sides = 18, n_cap = n_sides + 1;
			const float _2pi = PI * 2.0f;

			std::vector<Vec3> vertices(n_cap + n_cap + n_sides * 2 + 2);
			int vert = 0;

			vertices[vert++] = Vec3(0.0f, 0.0f, 0.0f);
			while(vert <= n_sides) {
				float rad = (float)vert / n_sides * _2pi;
				vertices[vert] = Vec3(std::cos(rad) * bradius, 0.0f, std::sin(rad) * bradius);
				vert++;
			}
			vertices[vert++] = Vec3(0.0f, height, 0.0f);
			while(vert <= n_sides * 2 + 1) {
				float rad = (float)(vert - n_sides - 1)  / n_sides * _2pi;
				vertices[vert] = Vec3(std::cos(rad) * tradius, height, std::sin(rad) * tradius);
				vert++;
			}
			int v = 0;
			while(vert <= vertices.size() - 4) {
				float rad = (float)v / n_sides * _2pi;
				vertices[vert] = Vec3(std::cos(rad) * tradius, height, std::sin(rad) * tradius);
				vertices[vert + 1] = Vec3(std::cos(rad) * bradius, 0.0f, std::sin(rad) * bradius);
				vert+=2;
				v++;
			}
			vertices[vert] = vertices[n_sides * 2 + 2];
			vertices[vert + 1] = vertices[n_sides * 2 + 3];

			std::vector<Vec3> normals(vertices.size());
			vert = 0;
			while(vert <= n_sides) {
				normals[vert++] = Vec3(0.0f, -1.0f, 0.0f);
			}
			while(vert <= n_sides * 2 + 1) {
				normals[vert++] = Vec3(0.0f, 1.0f, 0.0f);
			}
			
			v = 0;
			while(vert <= vertices.size() - 4) {
				float rad = (float)v / n_sides * _2pi;
				float cos = std::cos(rad);
				float sin = std::sin(rad);
				normals[vert] = Vec3(cos, 0.0f, sin);
				normals[vert+1] = normals[vert];
				vert+=2;
				v++;
			}
			normals[vert] = normals[n_sides * 2 + 2 ];
			normals[vert + 1] = normals[n_sides * 2 + 3 ];
			
			int n_tris = n_sides + n_sides + n_sides * 2;
			std::vector<int> triangles(n_tris * 3 + 3);
			
			int tri = 0;
			int i = 0;
			while (tri < n_sides - 1) {
				triangles[i] = 0;
				triangles[i + 1] = tri + 1;
				triangles[i + 2] = tri + 2;
				tri++;
				i += 3;
			}
			triangles[i] = 0;
			triangles[i + 1] = tri + 1;
			triangles[i + 2] = 1;
			tri++;
			i += 3;
			
			while(tri < n_sides * 2) {
				triangles[i] = tri + 2;
				triangles[i + 1] = tri + 1;
				triangles[i + 2] = n_cap;
				tri++;
				i += 3;
			}
			triangles[i] = n_cap + 1;
			triangles[i + 1] = tri + 1;
			triangles[i + 2] = n_cap;
			tri++;
			i += 3;
			tri++;

			while(tri <= n_tris) {
				triangles[i] = tri + 2;
				triangles[i + 1] = tri + 1;
				triangles[i + 2] = tri + 0;
				tri++;
				i += 3;
				triangles[i] = tri + 1;
				triangles[i + 1] = tri + 2;
				triangles[i + 2] = tri + 0;
				tri++;
				i += 3;
			}

			std::vector<GL::Mesh::Vert> verts;
			for(int i : triangles) {
				verts.push_back({vertices[i], normals[i]});
			}
			return verts;
		}

		std::vector<GL::Mesh::Vert> torus_verts(float iradius, float oradius) {

			const int n_rad_sides = 48, n_sides = 24;
			const float _2pi = PI * 2.0f;
			iradius = oradius - iradius;
			
			std::vector<Vec3> vertices((n_rad_sides+1) * (n_sides+1));
			for(int seg = 0; seg <= n_rad_sides; seg++) {
				
				int cur_seg = seg  == n_rad_sides ? 0 : seg;
			
				float t1 = (float)cur_seg / n_rad_sides * _2pi;
				Vec3 r1(std::cos(t1) * oradius, 0.0f, std::sin(t1) * oradius);
			
				for(int side = 0; side <= n_sides; side++) {
			
					int cur_side = side == n_sides ? 0 : side;
					Vec3 normale = cross(r1, {0.0f, 1.0f, 0.0f});
					float t2 = (float)cur_side / n_sides * _2pi;
					Vec3 r2 = Mat4::rotate(Degrees(-t1), {0.0f, 1.0f, 0.0f}) * Vec3(std::sin(t2) * iradius, std::cos(t2) * iradius, 0.0f);
			
					vertices[side + seg * (n_sides+1)] = r1 + r2;
				}
			}

			std::vector<Vec3> normals(vertices.size());
			for(int seg = 0; seg <= n_rad_sides; seg++) {
			
				int cur_seg = seg  == n_rad_sides ? 0 : seg;
				float t1 = (float)cur_seg / n_rad_sides * _2pi;
				Vec3 r1(std::cos(t1) * oradius, 0.0f, std::sin(t1) * oradius);
			
				for(int side = 0; side <= n_sides; side++) {
					normals[side + seg * (n_sides+1)] = (vertices[side + seg * (n_sides+1)] - r1).unit();
				}
			}

			int n_faces = vertices.size();
			int n_tris = n_faces * 2;
			int n_idx = n_tris * 3;
			std::vector<int> triangles(n_idx);
			
			int i = 0;
			for(int seg = 0; seg <= n_rad_sides; seg++) {
				for(int side = 0; side <= n_sides - 1; side++) {

					int current = side + seg * (n_sides+1);
					int next = side + (seg < (n_rad_sides) ?(seg+1) * (n_sides+1) : 0);
			
					if(i < triangles.size() - 6) {
						triangles[i++] = current;
						triangles[i++] = next;
						triangles[i++] = next+1;
						triangles[i++] = current;
						triangles[i++] = next+1;
						triangles[i++] = current+1;
					}
				}
			}

			std::vector<GL::Mesh::Vert> verts;
			for(int i : triangles) {
				verts.push_back({vertices[i], normals[i]});
			}
			return verts;
		}
	}
}
