
#pragma once

#include "../platform/gl.h"

#include <string>

namespace Util {

	GL::Mesh cube_mesh(float radius);
	GL::Mesh square_mesh(float radius);
	GL::Mesh cyl_mesh(float radius, float height);
	GL::Mesh torus_mesh(float iradius, float oradius);
	GL::Mesh sphere_mesh(float r);
	GL::Mesh cone_mesh(float bradius, float tradius, float height);

	GL::Mesh arrow_mesh();
	GL::Mesh scale_mesh();

	std::string obj_mesh(std::string obj_file, GL::Mesh& mesh);

	namespace Detail {
		std::vector<GL::Mesh::Vert> cube_verts(float r);
		std::vector<GL::Mesh::Vert> square_verts(float r);

		// https://wiki.unity3d.com/index.php/ProceduralPrimitives
		std::vector<GL::Mesh::Vert> ico_sphere_verts(float radius, int level);
		std::vector<GL::Mesh::Vert> cone_verts(float bradius, float tradius, float height);
		std::vector<GL::Mesh::Vert> torus_verts(float iradius, float oradius);
	}
}
