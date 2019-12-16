
#pragma once

#include "math.h"

class Camera {
public:
	Camera(Vec2 dim);
	~Camera();

	Mat4 view() const;
	Mat4 proj() const;

	void reset();
	void mouse(Vec2 off);
	void set_ar(Vec2 dim);

private:
	void update_dirs();

	// parameters
	float pitch, yaw, fov, n, ar, sens;
	Vec3 pos, global_up;

	// set by update_dirs
	Vec3 front, up, right;
};
