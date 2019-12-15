
#pragma once

#include "math.h"

struct Camera {

public:
    Camera() {}
    ~Camera() {}

	Mat4 view() {
		Mat4 ret = look_at(pos, pos + front, up);
		return ret;
	}

	void update_dirs() {
		front.x = std::cos(Radians(pitch)) * std::cos(Radians(yaw));
		front.y = sin(Radians(pitch));
		front.z = sin(Radians(yaw)) * std::cos(Radians(pitch));
		front.normalize();
		right = cross(front, global_up).unit();
		up = cross(right, front).unit();
	}

	void reset() {
		fov = 80.0f;
		pitch = -45.0f;
		yaw = 225.0f;
		speed = 5.0f;
		pos = Vec3(5, 5, 5);
		global_up = Vec3(0, 1, 0);
		update_dirs();
	}

	void move(int dx, int dy) {
		const float sens = 0.1f;
		yaw += dx * sens;
		pitch -= dy * sens;
		if (yaw > 360.0f) yaw = 0.0f;
		else if (yaw < 0.0f) yaw = 360.0f;
		if (pitch > 89.0f) pitch = 89.0f;
		else if (pitch < -89.0f) pitch = -89.0f;
		update_dirs();
	}

private:
	Vec3 pos, front, up, right, global_up;
	float pitch, yaw, speed, fov;
};
