
#pragma once

#include <cmath>
#include <algorithm>

#include "vec3.h"

struct Line {

	Line() {
	}
	Line(Vec3 point, Vec3 dir) :
        point(point),
        dir(dir) {
	}
	Line(const Line& src) {
		point = src.point;
		dir = src.dir;
	}

	Line operator=(Line v) {
		point = v.point;
		dir = v.dir;
		return *this;
	}

    Vec3 closest(Line other) const {
        Vec3 p0 = point - other.point;
        float a = dot(dir, other.dir);
        float b = dot(dir, p0);
        float c = dot(other.dir, p0);
        float t = (a*c - b) / (1.0f - a*a);
        return point + t * dir;
    }

    Vec3 point, dir;
};
