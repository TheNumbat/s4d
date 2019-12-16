
#pragma once

#include "../lib/math.h"
#include "../lib/gl.h"
#include "../lib/camera.h"

#include "scene_object.h"

#include <map>

class Scene {
public:
    Scene(Vec2 window_dim);
    ~Scene();

    void render();
    void gui();

    void apply_window_dim(Vec2 new_dim);
    void add_object(Scene_Object&& obj);

    void camera_orbit(Vec2 dmouse);
    void camera_move(Vec2 dmouse);
    void camera_radius(float dmouse);

    void reload_shaders();

private:
    Vec2 window_dim;
    Camera camera;
    GL_Shader mesh_shader;

    std::map<Scene_Object::ID, Scene_Object> objs;
    Scene_Object::ID next_id = 1;
    Scene_Object::ID selected_id = 0;
};

