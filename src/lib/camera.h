
#pragma once

#include "math.h"

class Camera {
public:
	Camera(Vec2 dim);
	~Camera();

	/// View transformation matrix
	Mat4 view() const;
	/// Perspective projection transformation matrix
	Mat4 proj() const;
	
	/// Camera position
	Vec3 pos() const;
	/// Camera look direction
	Vec3 front() const;

	/// Reset to default values
	void reset();
	
	/// Apply movement delta to orbit position
	void mouse_orbit(Vec2 off);
	/// Apply movement delta to look point
	void mouse_move(Vec2 off);
	/// Apply movement delta to radius (distance from look point)
	void mouse_radius(float off);

	/// Apply screen aspect ratio (for perspective projection)
	void set_ar(Vec2 dim);

private:
	void update_pos();

	// parameters
	float pitch, yaw, fov, n, ar, radius;
	float orbit_sens, move_sens, radius_sens;
	Vec3 global_up, center;

	// set by update_pos
	Vec3 _pos;
};
