
#include "proc_objects.h"

namespace Proc_Objects {

	GL::Mesh cube(float r) {

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
