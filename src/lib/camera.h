
#pragma once

#include "math.h"

class Camera {
public:
	Camera(Vec2 dim);
	~Camera();

	Mat4 view() const;
	Mat4 proj() const;

	void reset();
	
	void mouse_orbit(Vec2 off);
	void mouse_move(Vec2 off);
	void mouse_radius(float off);

	void set_ar(Vec2 dim);

private:
	void update_pos();

	// parameters
	float pitch, yaw, fov, n, ar, radius;
	float orbit_sens, move_sens, radius_sens;
	Vec3 global_up, center;

	// set by update_pos
	Vec3 pos;
};
