
#pragma once

#include <cmath>
#include <algorithm>

#include "log.h"

struct Vec2 {

	Vec2() {
		x = 0.0f;
		y = 0.0f;
	}
	Vec2(float _x, float _y) {
		x = _x;
		y = _y;
	}
	Vec2(float f) {
		x = y = f;
	}
	Vec2(int _x, int _y) {
		x = (float)_x;
		y = (float)_y;
	}
	Vec2(const Vec2& src) {
		x = src.x;
		y = src.y;
	}

	Vec2 operator=(Vec2 v) {
		x = v.x;
		y = v.y;
		return *this;
	}

	float& operator[](int idx) {
		assert(idx >= 0 && idx <= 1);
		return data[idx];
	}
	float operator[](int idx) const {
		assert(idx >= 0 && idx <= 1);
		return data[idx];
	}

	Vec2 operator+=(Vec2 v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	Vec2 operator-=(Vec2 v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	Vec2 operator*=(Vec2 v) {
		x *= v.x;
		y *= v.y;
		return *this;
	}
	Vec2 operator/=(Vec2 v) {
		x /= v.x;
		y /= v.y;
		return *this;
	}

	Vec2 operator+=(float s) {
		x += s;
		y += s;
		return *this;
	}
	Vec2 operator-=(float s) {
		x -= s;
		y -= s;
		return *this;
	}
	Vec2 operator*=(float s) {
		x *= s;
		y *= s;
		return *this;
	}
	Vec2 operator/=(float s) {
		x /= s;
		y /= s;
		return *this;
	}

	Vec2 operator+(Vec2 v) const {
		return Vec2(x + v.x, y + v.y);
	}
	Vec2 operator-(Vec2 v) const {
		return Vec2(x - v.x, y - v.y);
	}
	Vec2 operator*(Vec2 v) const {
		return Vec2(x * v.x, y * v.y);
	}
	Vec2 operator/(Vec2 v) const {
		return Vec2(x / v.x, y / v.y);
	}

	Vec2 operator+(float s) const {
		return Vec2(x + s, y + s);
	}
	Vec2 operator-(float s) const {
		return Vec2(x - s, y - s);
	}
	Vec2 operator*(float s) const {
		return Vec2(x * s, y * s);
	}
	Vec2 operator/(float s) const {
		return Vec2(x / s, y / s);
	}

	bool operator==(Vec2 v) const {
		return x == v.x && y == v.y;
	}
	bool operator!=(Vec2 v) const {
		return x != v.x || y != v.y;
	}

	Vec2 abs() const {
		return Vec2(std::abs(x), std::abs(y));
	}
	Vec2 operator-() const {
		return Vec2(-x, -y);
	}

	/// Modify vec to have unit length
	Vec2 normalize() {
		float n = norm();
		x /= n;
		y /= n;
		return *this;
	}
	/// Return unit length vec in the same direction
	Vec2 unit() const {
		float n = norm();
		return Vec2(x / n, y / n);
	}

	float norm_squared() const {
		return x * x + y * y;
	}
	float norm() const {
		return std::sqrt(norm_squared());
	}

	union {
		struct {
			float x;
			float y;
		};
		float data[2] = {};
	};
};

inline Vec2 operator+(float s, Vec2 v) {
	return Vec2(v.x + s, v.y + s);
}
inline Vec2 operator-(float s, Vec2 v) {
	return Vec2(v.x - s, v.y - s);
}
inline Vec2 operator*(float s, Vec2 v) {
	return Vec2(v.x * s, v.y * s);
}
inline Vec2 operator/(float s, Vec2 v) {
	return Vec2(s / v.x, s / v.y);
}

inline Vec2 hmin(Vec2 l, Vec2 r) {
	return Vec2(std::min(l.x, r.x), std::min(l.y, r.y));
}
inline Vec2 hmax(Vec2 l, Vec2 r) {
	return Vec2(std::max(l.x, r.x), std::max(l.y, r.y));
}

inline float dot(Vec2 l, Vec2 r) {
	return l.x * r.x + l.y * r.y;
}
