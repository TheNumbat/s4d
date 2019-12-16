
#include "camera.h"

Camera::Camera(Vec2 dim) {
    reset();
    set_ar(dim);
}

Camera::~Camera() {

}

Mat4 Camera::view() const {
    return Mat4::look_at(pos, pos + front, up);
}

Mat4 Camera::proj() const {
    return Mat4::project(fov, ar, n);
}

void Camera::set_ar(Vec2 dim) {
    ar = dim.x / dim.y;
}

void Camera::update_dirs() {
    front.x = std::cos(Radians(pitch)) * std::cos(Radians(yaw));
    front.y = std::sin(Radians(pitch));
    front.z = std::sin(Radians(yaw)) * std::cos(Radians(pitch));
    front.normalize();
    right = cross(front, global_up).unit();
    up = cross(right, front).unit();
}

void Camera::reset() {
    fov = 90.0f;
    pitch = -45.0f;
    yaw = 225.0f;
    n = 0.1f;
    sens = 0.1f;
    pos = Vec3(5, 5, 5);
    global_up = Vec3(0, 1, 0);
    update_dirs();
}

void Camera::mouse(Vec2 off) {
    int dx = (int)off.x;
    int dy = (int)off.y;
    yaw += dx * sens;
    pitch -= dy * sens;
    if (yaw > 360.0f) yaw = 0.0f;
    else if (yaw < 0.0f) yaw = 360.0f;
    if (pitch > 89.0f) pitch = 89.0f;
    else if (pitch < -89.0f) pitch = -89.0f;
    update_dirs();
}
