
#pragma once

#include <cmath>
#include <algorithm>

#include "log.h"
#include "vec4.h"

#ifndef PI
#define PI 3.14159265358979323846264338327950288f
#endif
#ifndef Radians
#define Radians(v) (v * (PI / 180.0f)) 
#endif

struct Mat4 {

	/// Identity matrix
	static const Mat4 I;
	/// Zero matrix
	static const Mat4 Zero;

	/// Return transposed matrix
	static Mat4 transpose(Mat4 m);
	/// Return inverse matrix (will be NaN if m is not invertible)
	static Mat4 inverse(Mat4 m);
	/// Return tranformation matrix for given translation vector
	static Mat4 translate(Vec3 t);
	/// Return tranformation matrix for given angle (degrees) and axis
	static Mat4 rotate(float t, Vec3 axis);
	/// Return transformation matrix for given scale factors
	static Mat4 scale(Vec3 s);

	/// Return transformation matrix for viewing a scene from $pos looking at $at,
	/// where straight up is defined as $up
	static Mat4 look_at(Vec3 pos, Vec3 at, Vec3 up = {0.0f, 1.0f, 0.0f});
	/// Return orthogonal projection matrix with given left, right, bottom, top,
	/// near, and far planes.
	static Mat4 ortho(float l, float r, float b, float t, float n, float f);
	/// Return perspective projection matrix with given field of view, aspect ratio,
	/// and near plane. The far plane is assumed to be at infinity. 
	static Mat4 project(float fov, float ar, float n);

	Mat4() {
		*this = I;
	}
	Mat4(Vec4 x, Vec4 y, Vec4 z, Vec4 w) {
		cols[0] = x;
		cols[1] = y;
		cols[2] = z;
		cols[3] = w;
	}
	Mat4(const Mat4& src) {
		for(int i = 0; i < 4; i++)
			cols[i] = src.cols[i];
	}

	Mat4 operator=(Mat4 m) {
		for(int i = 0; i < 4; i++)
			cols[i] = m.cols[i];
		return *this;
	}

	Vec4& operator[](int idx) {
		assert(idx >= 0 && idx <= 3);
		return cols[idx];
	}
	Vec4 operator[](int idx) const {
		assert(idx >= 0 && idx <= 3);
		return cols[idx];
	}

	Mat4 operator+=(Mat4 m) {
		for(int i = 0; i < 4; i++)
			cols[i] += m.cols[i];
		return *this;
	}
	Mat4 operator-=(Mat4 m) {
		for(int i = 0; i < 4; i++)
			cols[i] -= m.cols[i];
		return *this;
	}

	Mat4 operator+=(float s) {
		for(int i = 0; i < 4; i++)
			cols[i] += s;
		return *this;
	}
	Mat4 operator-=(float s) {
		for(int i = 0; i < 4; i++)
			cols[i] -= s;
		return *this;
	}
	Mat4 operator*=(float s) {
		for(int i = 0; i < 4; i++)
			cols[i] *= s;
		return *this;
	}
	Mat4 operator/=(float s) {
		for(int i = 0; i < 4; i++)
			cols[i] /= s;
		return *this;
	}


	Mat4 operator+(Mat4 m) const {
		Mat4 r;
		for(int i = 0; i < 4; i++)
			r.cols[i] = cols[i] + m.cols[i];
		return r;
	}
	Mat4 operator-(Mat4 m) const {
		Mat4 r;
		for(int i = 0; i < 4; i++)
			r.cols[i] = cols[i] - m.cols[i];
		return r;
	}

	Mat4 operator+(float s) const {
		Mat4 r;
		for(int i = 0; i < 4; i++)
			r.cols[i] = cols[i] + s;
		return r;
	}
	Mat4 operator-(float s) const {
		Mat4 r;
		for(int i = 0; i < 4; i++)
			r.cols[i] = cols[i] - s;
		return r;
	}
	Mat4 operator*(float s) const {
		Mat4 r;
		for(int i = 0; i < 4; i++)
			r.cols[i] = cols[i] * s;
		return r;
	}
	Mat4 operator/(float s) const {
		Mat4 r;
		for(int i = 0; i < 4; i++)
			r.cols[i] = cols[i] / s;
		return r;
	}

	Mat4 operator*=(Mat4 v) {
		*this = *this * v;
		return *this;
	}
	Mat4 operator*(Mat4 m) const {
		Mat4 ret;
		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++) {
				ret[i][j] = 0.0f;
				for(int k = 0; k < 4; k++) {
					ret[i][j] += cols[i][k] * m[k][j];
				}
			}
		}
		return ret;
	}

	Vec4 operator*(Vec4 v) const {
		return v[0] * cols[0] + v[1] * cols[1] +
			   v[2] * cols[2] + v[3] * cols[3];
	}

	/// Expands v to Vec4(v, 1.0), multiplies, and projects back to 3D
	Vec3 operator*(Vec3 v) const {
		return operator*(Vec4(v, 1.0f)).project();
	}
	/// Expands v to Vec4(v, 0.0), multiplies, and projects back to 3D
	Vec3 rotate(Vec3 v) const {
		return operator*(Vec4(v, 0.0f)).project();
	}

	/// Returns matrix transpose
	Mat4 T() const {
		return transpose(*this);
	}
	/// Returns matrix inverse (will be NaN if m is not invertible)
	Mat4 inverse() const {
		return inverse(*this);
	}

	/// Returns determinant (brute force).
	float det() const {
		return cols[0][3]*cols[1][2]*cols[2][1]*cols[3][0] - cols[0][2]*cols[1][3]*cols[2][1]*cols[3][0] -
		cols[0][3]*cols[1][1]*cols[2][2]*cols[3][0] + cols[0][1]*cols[1][3]*cols[2][2]*cols[3][0] +
		cols[0][2]*cols[1][1]*cols[2][3]*cols[3][0] - cols[0][1]*cols[1][2]*cols[2][3]*cols[3][0] -
		cols[0][3]*cols[1][2]*cols[2][0]*cols[3][1] + cols[0][2]*cols[1][3]*cols[2][0]*cols[3][1] +
		cols[0][3]*cols[1][0]*cols[2][2]*cols[3][1] - cols[0][0]*cols[1][3]*cols[2][2]*cols[3][1] -
		cols[0][2]*cols[1][0]*cols[2][3]*cols[3][1] + cols[0][0]*cols[1][2]*cols[2][3]*cols[3][1] +
		cols[0][3]*cols[1][1]*cols[2][0]*cols[3][2] - cols[0][1]*cols[1][3]*cols[2][0]*cols[3][2] -
		cols[0][3]*cols[1][0]*cols[2][1]*cols[3][2] + cols[0][0]*cols[1][3]*cols[2][1]*cols[3][2] +
		cols[0][1]*cols[1][0]*cols[2][3]*cols[3][2] - cols[0][0]*cols[1][1]*cols[2][3]*cols[3][2] -
		cols[0][2]*cols[1][1]*cols[2][0]*cols[3][3] + cols[0][1]*cols[1][2]*cols[2][0]*cols[3][3] +
		cols[0][2]*cols[1][0]*cols[2][1]*cols[3][3] - cols[0][0]*cols[1][2]*cols[2][1]*cols[3][3] -
		cols[0][1]*cols[1][0]*cols[2][2]*cols[3][3] + cols[0][0]*cols[1][1]*cols[2][2]*cols[3][3];
	}

	union {
		Vec4 cols[4];
		float data[16] = {};
	};
};

inline Mat4 operator+(float s, Mat4 m) {
	Mat4 r;
	for(int i = 0; i < 4; i++)
		r.cols[i] = m.cols[i] + s;
	return r;
}
inline Mat4 operator-(float s, Mat4 m) {
	Mat4 r;
	for(int i = 0; i < 4; i++)
		r.cols[i] = m.cols[i] - s;
	return r;
}
inline Mat4 operator*(float s, Mat4 m) {
	Mat4 r;
	for(int i = 0; i < 4; i++)
		r.cols[i] = m.cols[i] * s;
	return r;
}
inline Mat4 operator/(float s, Mat4 m) {
	Mat4 r;
	for(int i = 0; i < 4; i++)
		r.cols[i] = m.cols[i] / s;
	return r;
}

const inline Mat4 Mat4::I = {{1.0f, 0.0f, 0.0f, 0.0f}, 
							 {0.0f, 1.0f, 0.0f, 0.0f},
							 {0.0f, 0.0f, 1.0f, 0.0f},
							 {0.0f, 0.0f, 0.0f, 1.0f}};
const inline Mat4 Mat4::Zero = {{0.0f, 0.0f, 0.0f, 0.0f}, 
								{0.0f, 0.0f, 0.0f, 0.0f},
								{0.0f, 0.0f, 0.0f, 0.0f},
								{0.0f, 0.0f, 0.0f, 0.0f}};

inline Mat4 Mat4::transpose(Mat4 m) {
	Mat4 r;
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			r[i][j] = m[j][i];
		}
	}
	return r;
}

inline Mat4 Mat4::inverse(Mat4 m) {
	Mat4 r;
	r[0][0] = m[1][2]*m[2][3]*m[3][1] - m[1][3]*m[2][2]*m[3][1] + m[1][3]*m[2][1]*m[3][2] - m[1][1]*m[2][3]*m[3][2] - m[1][2]*m[2][1]*m[3][3] + m[1][1]*m[2][2]*m[3][3];
	r[0][1] = m[0][3]*m[2][2]*m[3][1] - m[0][2]*m[2][3]*m[3][1] - m[0][3]*m[2][1]*m[3][2] + m[0][1]*m[2][3]*m[3][2] + m[0][2]*m[2][1]*m[3][3] - m[0][1]*m[2][2]*m[3][3];
	r[0][2] = m[0][2]*m[1][3]*m[3][1] - m[0][3]*m[1][2]*m[3][1] + m[0][3]*m[1][1]*m[3][2] - m[0][1]*m[1][3]*m[3][2] - m[0][2]*m[1][1]*m[3][3] + m[0][1]*m[1][2]*m[3][3];
	r[0][3] = m[0][3]*m[1][2]*m[2][1] - m[0][2]*m[1][3]*m[2][1] - m[0][3]*m[1][1]*m[2][2] + m[0][1]*m[1][3]*m[2][2] + m[0][2]*m[1][1]*m[2][3] - m[0][1]*m[1][2]*m[2][3];
	r[1][0] = m[1][3]*m[2][2]*m[3][0] - m[1][2]*m[2][3]*m[3][0] - m[1][3]*m[2][0]*m[3][2] + m[1][0]*m[2][3]*m[3][2] + m[1][2]*m[2][0]*m[3][3] - m[1][0]*m[2][2]*m[3][3];
	r[1][1] = m[0][2]*m[2][3]*m[3][0] - m[0][3]*m[2][2]*m[3][0] + m[0][3]*m[2][0]*m[3][2] - m[0][0]*m[2][3]*m[3][2] - m[0][2]*m[2][0]*m[3][3] + m[0][0]*m[2][2]*m[3][3];
	r[1][2] = m[0][3]*m[1][2]*m[3][0] - m[0][2]*m[1][3]*m[3][0] - m[0][3]*m[1][0]*m[3][2] + m[0][0]*m[1][3]*m[3][2] + m[0][2]*m[1][0]*m[3][3] - m[0][0]*m[1][2]*m[3][3];
	r[1][3] = m[0][2]*m[1][3]*m[2][0] - m[0][3]*m[1][2]*m[2][0] + m[0][3]*m[1][0]*m[2][2] - m[0][0]*m[1][3]*m[2][2] - m[0][2]*m[1][0]*m[2][3] + m[0][0]*m[1][2]*m[2][3];
	r[2][0] = m[1][1]*m[2][3]*m[3][0] - m[1][3]*m[2][1]*m[3][0] + m[1][3]*m[2][0]*m[3][1] - m[1][0]*m[2][3]*m[3][1] - m[1][1]*m[2][0]*m[3][3] + m[1][0]*m[2][1]*m[3][3];
	r[2][1] = m[0][3]*m[2][1]*m[3][0] - m[0][1]*m[2][3]*m[3][0] - m[0][3]*m[2][0]*m[3][1] + m[0][0]*m[2][3]*m[3][1] + m[0][1]*m[2][0]*m[3][3] - m[0][0]*m[2][1]*m[3][3];
	r[2][2] = m[0][1]*m[1][3]*m[3][0] - m[0][3]*m[1][1]*m[3][0] + m[0][3]*m[1][0]*m[3][1] - m[0][0]*m[1][3]*m[3][1] - m[0][1]*m[1][0]*m[3][3] + m[0][0]*m[1][1]*m[3][3];
	r[2][3] = m[0][3]*m[1][1]*m[2][0] - m[0][1]*m[1][3]*m[2][0] - m[0][3]*m[1][0]*m[2][1] + m[0][0]*m[1][3]*m[2][1] + m[0][1]*m[1][0]*m[2][3] - m[0][0]*m[1][1]*m[2][3];
	r[3][0] = m[1][2]*m[2][1]*m[3][0] - m[1][1]*m[2][2]*m[3][0] - m[1][2]*m[2][0]*m[3][1] + m[1][0]*m[2][2]*m[3][1] + m[1][1]*m[2][0]*m[3][2] - m[1][0]*m[2][1]*m[3][2];
	r[3][1] = m[0][1]*m[2][2]*m[3][0] - m[0][2]*m[2][1]*m[3][0] + m[0][2]*m[2][0]*m[3][1] - m[0][0]*m[2][2]*m[3][1] - m[0][1]*m[2][0]*m[3][2] + m[0][0]*m[2][1]*m[3][2];
	r[3][2] = m[0][2]*m[1][1]*m[3][0] - m[0][1]*m[1][2]*m[3][0] - m[0][2]*m[1][0]*m[3][1] + m[0][0]*m[1][2]*m[3][1] + m[0][1]*m[1][0]*m[3][2] - m[0][0]*m[1][1]*m[3][2];
	r[3][3] = m[0][1]*m[1][2]*m[2][0] - m[0][2]*m[1][1]*m[2][0] + m[0][2]*m[1][0]*m[2][1] - m[0][0]*m[1][2]*m[2][1] - m[0][1]*m[1][0]*m[2][2] + m[0][0]*m[1][1]*m[2][2];
	r /= m.det();
	return r;
}

inline Mat4 Mat4::translate(Vec3 t) {
	Mat4 r;
	r[3] = Vec4(t, 1.0f);
	return r;
}

inline Mat4 Mat4::rotate(float t, Vec3 axis) {
	Mat4 ret;
	float c = cosf(Radians(t));
	float s = sinf(Radians(t));
	axis.normalize();
	Vec3 temp = axis * (1.0f - c);
	ret[0][0] = c + temp[0] * axis[0];
	ret[0][1] = temp[0] * axis[1] + s * axis[2];
	ret[0][2] = temp[0] * axis[2] - s * axis[1];
	ret[1][0] = temp[1] * axis[0] - s * axis[2];
	ret[1][1] = c + temp[1] * axis[1];
	ret[1][2] = temp[1] * axis[2] + s * axis[0];
	ret[2][0] = temp[2] * axis[0] + s * axis[1];
	ret[2][1] = temp[2] * axis[1] - s * axis[0];
	ret[2][2] = c + temp[2] * axis[2];
	return ret;
}

inline Mat4 Mat4::scale(Vec3 s) {
	Mat4 r;
	r[0][0] = s.x;
	r[1][1] = s.y;
	r[2][2] = s.z;
	return r;
}

inline Mat4 Mat4::ortho(float l, float r, float b, float t, float n, float f) {
	Mat4 rs;
	rs[0][0] = 2.0f / (r - l);
	rs[1][1] = 2.0f / (t - b);
	rs[2][2] = 2.0f / (n - f);
	rs[3][0] = (-l - r) / (r - l);
	rs[3][1] = (-b - t)  / (t - b);
	rs[3][2] = - n / (f - n);
	return rs;
}

inline Mat4 Mat4::project(float fov, float ar, float n) {
	float f = 1.0f / std::tanf(Radians(fov) / 2.0f);
	Mat4 r;
	r[0][0] = f / ar;
	r[1][1] = f;
	r[2][2] = 0.0f;
	r[3][3] = 0.0f;
	r[3][2] = n;
	r[2][3] = -1.0f;
	return r;
}

inline Mat4 Mat4::look_at(Vec3 pos, Vec3 at, Vec3 up) {
	Mat4 r = Mat4::Zero;
	Vec3 F = (at - pos).unit();
	Vec3 S = cross(F, up).unit();
	Vec3 U = cross(S, F);
	r[0][0] =  S.x;
	r[0][1] =  U.x;
	r[0][2] = -F.x;
	r[1][0] =  S.y;
	r[1][1] =  U.y;
	r[1][2] = -F.y;
	r[2][0] =  S.z;
	r[2][1] =  U.z;
	r[2][2] = -F.z;
	r[3][0] = -dot(S, pos);
	r[3][1] = -dot(U, pos);
	r[3][2] =  dot(F, pos);
	r[3][3] = 1.0f;
	return r;
}
