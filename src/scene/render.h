
#pragma once

#include "../platform/gl.h"
#include "scene.h"

class Renderer {
public:
    static void setup(Vec2 dim);
    static void shutdown();
    
    static void begin();
    static void complete();
    static void reset_depth();
    
    static void proj(Mat4 proj);
    static void update_dim(Vec2 dim);
    static void settings_gui(bool* open);
    static Scene_Object::ID read_id(Vec2 pos);

    struct MeshOpt {
        Scene_Object::ID id;
        Mat4 modelview, normal;
        Vec3 color;
        bool wireframe = false;
        bool solid_color = false;
        bool depth_only = false;
        bool per_vert_id = false;
    }; 

    static void mesh(const GL::Mesh& mesh, MeshOpt opt);
    static void lines(const GL::Lines& lines, Mat4 viewproj, float alpha);
    static void outline(Mat4 viewproj, Mat4 view, const Scene_Object& obj);

private:
	static inline GL::Framebuffer framebuffer, id_resolve;
    static inline GL::Shader mesh_shader, line_shader; 
    static inline Mat4 _proj;
    static inline int samples = 4;
    static inline GLubyte* id_buffer = nullptr;
    static inline Vec2 window_dim;
};
