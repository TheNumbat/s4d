
#include "camera.h"

Camera::Camera(Vec2 dim) {
	reset();
	set_ar(dim);
}

Camera::~Camera() {

}

Vec3 Camera::front() const {
	return (_pos - center).unit();
}

Vec3 Camera::pos() const {
	return _pos;
}

Mat4 Camera::view() const {
	return Mat4::look_at(_pos, center, global_up);
}

Mat4 Camera::proj() const {
	return Mat4::project(fov, ar, n);
}

void Camera::set_ar(Vec2 dim) {
	ar = dim.x / dim.y;
}

void Camera::update_pos() {
	_pos.x = std::cos(Radians(pitch)) * std::cos(Radians(yaw));
	_pos.y = std::sin(Radians(pitch));
	_pos.z = std::sin(Radians(yaw)) * std::cos(Radians(pitch));
	_pos = radius * _pos.unit() + center;
}

void Camera::reset() {
	fov = 90.0f;
	pitch = 45.0f;
	yaw = 45.0f;
	n = 0.01f;
	radius = 5.0f;
	radius_sens = 0.25f;
	move_sens = 0.015f;
	orbit_sens = 0.2f;
	center = Vec3();
	global_up = Vec3(0, 1, 0);
	update_pos();
}

void Camera::mouse_orbit(Vec2 off) {
	yaw += off.x * orbit_sens;
	pitch -= off.y * orbit_sens;
	if (yaw > 360.0f) yaw = 0.0f;
	else if (yaw < 0.0f) yaw = 360.0f;
	pitch = clamp(pitch, -89.0f, 89.0f);
	update_pos();
}

void Camera::mouse_move(Vec2 off) {
	Vec3 front = (_pos - center).unit();
	Vec3 right = cross(front, global_up).unit();
	Vec3 up = cross(front, right).unit();
	center += right * off.x * move_sens - up * off.y * move_sens;
	update_pos();
}

void Camera::mouse_radius(float off) {
	radius -= off * radius_sens;
	radius = std::max(radius, 2.0f * n);
	update_pos();
}
