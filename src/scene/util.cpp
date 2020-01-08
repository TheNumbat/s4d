
#include "util.h"

#include <map>

namespace Util {

	GL::Mesh cyl_mesh(float radius, float height) {
		return cone_mesh(radius, radius, height);
	}

	GL::Mesh arrow_mesh() {
		Gen::Data base = Gen::cone(0.03f, 0.03f, 0.7f);
		Gen::Data tip = Gen::cone(0.075f, 0.005f, 0.2f);
		for(auto& v : tip.verts) v.pos.y += 0.7f;
		for(auto& i : tip.elems) i += base.verts.size();
		base.verts.insert(base.verts.end(), tip.verts.begin(), tip.verts.end());
		base.elems.insert(base.elems.end(), tip.elems.begin(), tip.elems.end());
		return GL::Mesh(std::move(base.verts), std::move(base.elems));
	}

	GL::Mesh scale_mesh() {
		Gen::Data base = Gen::cone(0.03f, 0.03f, 0.7f);
		Gen::Data tip = Gen::cube(0.1f);
		for(auto& v : tip.verts) v.pos.y += 0.7f;
		for(auto& i : tip.elems) i += base.verts.size();
		base.verts.insert(base.verts.end(), tip.verts.begin(), tip.verts.end());
		base.elems.insert(base.elems.end(), tip.elems.begin(), tip.elems.end());
		return GL::Mesh(std::move(base.verts), std::move(base.elems));
	}
	
	GL::Mesh cone_mesh(float bradius, float tradius, float height) {
		Gen::Data cone = Gen::cone(bradius, tradius, height);
		return GL::Mesh(std::move(cone.verts), std::move(cone.elems));
	}

	GL::Mesh torus_mesh(float iradius, float oradius) {
		Gen::Data torus = Gen::torus(iradius, oradius);
		return GL::Mesh(std::move(torus.verts), std::move(torus.elems));
	}

	GL::Mesh cube_mesh(float r) {
		Gen::Data cube = Gen::cube(r);
		return GL::Mesh(std::move(cube.verts), std::move(cube.elems));
	}

	GL::Mesh square_mesh(float r) {
		Gen::Data square = Gen::square(r);
		return GL::Mesh(std::move(square.verts), std::move(square.elems));
	}

	GL::Mesh sphere_mesh(float r) {
		Gen::Data ico_sphere = Gen::ico_sphere(r, 3);
		return GL::Mesh(std::move(ico_sphere.verts), std::move(ico_sphere.elems));
	}

	namespace Gen {

		Data square(float r) {
			return {{
				{{-r, 0.0f, -r}, {0.0f, 1.0f, 0.0f}},
				{{-r, 0.0f, r}, {0.0f, 1.0f, 0.0f}},
				{{r, 0.0f, -r}, {0.0f, 1.0f, 0.0f}},
				{{r, 0.0f, r}, {0.0f, 1.0f, 0.0f}}},
				{0, 1, 2, 2, 1, 3}};
		}

		Data cube(float r) {
			return {{
				{{-r, -r, -r}, Vec3{-r, -r, -r}.unit()},
    			{{r, -r, -r}, Vec3{r, -r, -r}.unit()},
    			{{r, r, -r}, Vec3{r, r, -r}.unit()},
    			{{-r, r, -r}, Vec3{-r, r, -r}.unit()},
    			{{-r, -r, r}, Vec3{-r, -r, r}.unit()},
    			{{r, -r, r}, Vec3{r, -r, r}.unit()},
    			{{r, r, r}, Vec3{r, r, r}.unit()},
    			{{-r, r, r}, Vec3{-r, r, r}.unit()}},
				{0, 1, 3, 3, 1, 2,
				 1, 5, 2, 2, 5, 6,
				 5, 4, 6, 6, 4, 7,
				 4, 0, 7, 7, 0, 3,
				 3, 2, 7, 7, 2, 6,
				 4, 5, 0, 0, 5, 1}};
		}

		// https://wiki.unity3d.com/index.php/ProceduralPrimitives
		Data cone(float bradius, float tradius, float height) {

			const size_t n_sides = 18, n_cap = n_sides + 1;
			const float _2pi = PI * 2.0f;

			std::vector<Vec3> vertices(n_cap + n_cap + n_sides * 2 + 2);
			size_t vert = 0;

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
			
			size_t n_tris = n_sides + n_sides + n_sides * 2;
			std::vector<GL::Mesh::Index> triangles(n_tris * 3 + 3);
			
			size_t tri = 0;
			size_t i = 0;
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
			for(size_t i = 0; i < vertices.size(); i++) {
				verts.push_back({vertices[i], normals[i]});
			}
			return {verts, triangles};
		}

		Data torus(float iradius, float oradius) {

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
			std::vector<GL::Mesh::Index> triangles(n_idx);
			
			size_t i = 0;
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
			for(size_t i = 0; i < vertices.size(); i++) {
				verts.push_back({vertices[i], normals[i]});
			}
			return {verts, triangles};
		}

		Data ico_sphere(float radius, int level) {
			struct TriIdx {
				int v1, v2, v3;
			};
		
			auto middle_point = [&](int p1, int p2, std::vector<Vec3>& vertices, std::map<int64_t, size_t>& cache, float radius) -> size_t {
				bool firstIsSmaller = p1 < p2;
				int64_t smallerIndex = firstIsSmaller ? p1 : p2;
				int64_t greaterIndex = firstIsSmaller ? p2 : p1;
				int64_t key = (smallerIndex << 32ll) + greaterIndex;
				
				auto entry = cache.find(key);
				if (entry != cache.end()) {
					return entry->second;
				}
		
				Vec3 point1 = vertices[p1];
				Vec3 point2 = vertices[p2];
				Vec3 middle (
					(point1.x + point2.x) / 2.0f, 
					(point1.y + point2.y) / 2.0f, 
					(point1.z + point2.z) / 2.0f
				);
				size_t i = vertices.size();
				vertices.push_back(middle.unit() * radius); 
				cache[key] = i;
				return i;
			};
		
			std::vector<Vec3> vertices;
			std::map<int64_t, size_t> middlePointIndexCache;
			float t = (1.0f + std::sqrt(5.0f)) / 2.0f;
			vertices.push_back(Vec3(-1.0f,  t, 0.0f).unit() * radius);
			vertices.push_back(Vec3( 1.0f,  t, 0.0f).unit() * radius);
			vertices.push_back(Vec3(-1.0f, -t, 0.0f).unit() * radius);
			vertices.push_back(Vec3( 1.0f, -t, 0.0f).unit() * radius);
			vertices.push_back(Vec3(0.0f, -1.0f,  t).unit() * radius);
			vertices.push_back(Vec3(0.0f,  1.0f,  t).unit() * radius);
			vertices.push_back(Vec3(0.0f, -1.0f, -t).unit() * radius);
			vertices.push_back(Vec3(0.0f,  1.0f, -t).unit() * radius);
			vertices.push_back(Vec3( t, 0.0f, -1.0f).unit() * radius);
			vertices.push_back(Vec3( t, 0.0f,  1.0f).unit() * radius);
			vertices.push_back(Vec3(-t, 0.0f, -1.0f).unit() * radius);
			vertices.push_back(Vec3(-t, 0.0f,  1.0f).unit() * radius);
	
			std::vector<TriIdx> faces;
			faces.push_back(TriIdx{0, 11, 5});
			faces.push_back(TriIdx{0, 5, 1});
			faces.push_back(TriIdx{0, 1, 7});
			faces.push_back(TriIdx{0, 7, 10});
			faces.push_back(TriIdx{0, 10, 11});
			faces.push_back(TriIdx{1, 5, 9});
			faces.push_back(TriIdx{5, 11, 4});
			faces.push_back(TriIdx{11, 10, 2});
			faces.push_back(TriIdx{10, 7, 6});
			faces.push_back(TriIdx{7, 1, 8});
			faces.push_back(TriIdx{3, 9, 4});
			faces.push_back(TriIdx{3, 4, 2});
			faces.push_back(TriIdx{3, 2, 6});
			faces.push_back(TriIdx{3, 6, 8});
			faces.push_back(TriIdx{3, 8, 9});
			faces.push_back(TriIdx{4, 9, 5});
			faces.push_back(TriIdx{2, 4, 11});
			faces.push_back(TriIdx{6, 2, 10});
			faces.push_back(TriIdx{8, 6, 7});
			faces.push_back(TriIdx{9, 8, 1});
	
			for(int i = 0; i < level; i++) {
				std::vector<TriIdx> faces2;
				for(auto tri : faces) {
					int a = middle_point(tri.v1, tri.v2, vertices, middlePointIndexCache, radius);
					int b = middle_point(tri.v2, tri.v3, vertices, middlePointIndexCache, radius);
					int c = middle_point(tri.v3, tri.v1, vertices, middlePointIndexCache, radius);
					faces2.push_back(TriIdx{tri.v1, a, c});
					faces2.push_back(TriIdx{tri.v2, b, a});
					faces2.push_back(TriIdx{tri.v3, c, b});
					faces2.push_back(TriIdx{a, b, c});
				}
				faces = faces2;
			}
	
			std::vector<GL::Mesh::Index> triangles;
			for(size_t i = 0; i < faces.size(); i++ ) {
				triangles.push_back(faces[i].v1);
				triangles.push_back(faces[i].v2);
				triangles.push_back(faces[i].v3);
			}
	
			std::vector<Vec3> normals(vertices.size());
			for(size_t i = 0; i < normals.size(); i++)
				normals[i] = vertices[i].unit();
	
			std::vector<GL::Mesh::Vert> verts;
			for(size_t i = 0; i < vertices.size(); i++) {
				verts.push_back({vertices[i], normals[i]});
			}
			return {verts, triangles};
		}
	}
}
