
#pragma once

#include <cmath>
#include <algorithm>

#include "line.h"
#include "vec4.h"

struct Plane {

	Plane() {
	}
	Plane(Vec4 p) :
        p(p) {
	}
    Plane(Vec3 point, Vec3 n) {
        p.x = n.x;
        p.y = n.y;
        p.z = n.z;
        p.w = dot(point, n);
    }
	Plane(const Plane& src) {
		p = src.p;
	}

	Plane operator=(Plane v) {
		p = v.p;
		return *this;
	}

    bool hit(Line line, Vec3& pt) const {
        Vec3 n = p.xyz();
        float t = (p.w - dot(line.point, n)) / dot(line.dir, n);
        pt = line.at(t);
        return t >= 0.0f;
    }

    Vec4 p;
};
