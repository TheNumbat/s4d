
#pragma once

#include "../platform/gl.h"
#include "scene.h"

// Singleton
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
        Mat4 modelview;
        Vec3 color;
        bool wireframe = false;
        bool solid_color = false;
        bool depth_only = false;
        bool per_vert_id = false;
    }; 

    struct HalfedgeOpt {
        Mat4 modelview;
        Vec3 color;
    };

    static void mesh(const GL::Mesh& mesh, MeshOpt opt);
    static void halfedge(const GL::Mesh& faces, const Halfedge_Mesh& mesh, HalfedgeOpt opt);
    static void lines(const GL::Lines& lines, Mat4 viewproj, float alpha);
    static void outline(Mat4 viewproj, Mat4 view, const Scene_Object& obj);

private:
    void build_halfedge(const Halfedge_Mesh& mesh);

    Renderer(Vec2 dim);
    ~Renderer();
    static inline Renderer* data = nullptr;

    int samples;
    Vec2 window_dim;
    GLubyte* id_buffer;
	GL::Framebuffer framebuffer, id_resolve;
    GL::Shader mesh_shader, line_shader, inst_shader; 
    GL::Instances spheres, cylinders, arrows;
    const Halfedge_Mesh* loaded_mesh = nullptr;
    Mat4 _proj;
};
