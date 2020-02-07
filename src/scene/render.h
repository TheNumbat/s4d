
#pragma once

#include <variant>

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
        Vec3 color, sel_color, hov_color;
        unsigned int sel_id = 0, hov_id = 0;
        bool wireframe = false;
        bool solid_color = false;
        bool depth_only = false;
        bool per_vert_id = false;
    }; 

    struct HalfedgeOpt {
        Mat4 modelview;
        Vec3 color;
    };

    static void halfedge(const GL::Mesh& faces, const Halfedge_Mesh& mesh, HalfedgeOpt opt);
    
    static void set_he_select(unsigned int id);
    static void set_he_select(Halfedge_Mesh::ElementCRef elem);
    static void set_he_hover(Vec2 mouse);
    static unsigned int get_he_select();
    static std::optional<Halfedge_Mesh::ElementCRef> he_selected();

    static void mesh(const GL::Mesh& mesh, MeshOpt opt);
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
    
    Mat4 _proj;
    const Halfedge_Mesh* loaded_mesh = nullptr;
    
    // This all needs to be updated when the mesh connectivity changes
    unsigned int selected_compo = -1, hover_compo = -1;
    unsigned int faces = 0, verts = 0, edges = 0, halfedges = 0;

    // TODO(max): this is a bad design and would be un-necessary if we used
    // a halfedge implementation with contiguous indices. For now these must
    // be updated (along with the instance data) by build_halfedge whenever
    // the mesh changes its connectivity.
    std::map<Halfedge_Mesh::VertexCRef, unsigned int> v_to_idx;
    std::map<Halfedge_Mesh::EdgeCRef, unsigned int> e_to_idx;
    std::map<Halfedge_Mesh::HalfedgeCRef, unsigned int> he_to_idx;
    std::map<Halfedge_Mesh::FaceCRef, unsigned int> f_to_idx;
    std::map<unsigned int, Halfedge_Mesh::ElementCRef> idx_to_elm;
};
