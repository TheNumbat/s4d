
#include "proc_objects.h"

namespace Proc_Objects {

    GL_Mesh cube(float r) {

        std::vector<Vec3> verts = {
            Vec3(-r, -r,  r),
            Vec3(r, -r,  r),
            Vec3(r,  r,  r),
            Vec3(-r,  r,  r),
            Vec3(-r, -r, -r),
            Vec3(r, -r, -r),
            Vec3(r,  r, -r),
            Vec3(-r,  r, -r)};

        std::vector<unsigned int> elems = {
            0, 1, 2,
		    2, 3, 0,
		    1, 5, 6,
		    6, 2, 1,
		    7, 6, 5,
		    5, 4, 7,
		    4, 0, 3,
		    3, 7, 4,
		    4, 5, 1,
		    1, 0, 4,
		    3, 2, 6,
		    6, 7, 3};

        return GL_Mesh(verts, elems);
    }
}
