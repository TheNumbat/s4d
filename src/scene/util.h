
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

	namespace Gen {
		struct Data {
			std::vector<GL::Mesh::Vert> verts;
			std::vector<GL::Mesh::Index> elems;
		};

		Data cube(float r);
		Data square(float r);

		// https://wiki.unity3d.com/index.php/ProceduralPrimitives
		Data ico_sphere(float radius, int level);
		Data cone(float bradius, float tradius, float height);
		Data torus(float iradius, float oradius);
	}
}
