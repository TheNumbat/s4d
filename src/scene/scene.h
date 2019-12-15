
#pragma once

#include "../lib/math.h"
#include "../lib/gl.h"
#include "scene_object.h"

#include <map>

class Scene {
public:
    Scene();
    ~Scene();

    void render();
    void gui(Vec2 window_dim);

    void add_object(Scene_Object&& obj);

private:
    Mat4 view, proj;
    GL_Shader mesh_shader;

    std::map<Scene_Object::ID, Scene_Object> objs;
    Scene_Object::ID next_id = 1;
    Scene_Object::ID selected_id = 0;
};

