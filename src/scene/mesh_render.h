
#pragma once

#include <variant>

#include "../platform/gl.h"
#include "scene.h"
#include "../gui.h"

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

    // NOTE(max): updates & uses the indices in mesh for selection/traversal
    static void halfedge(Halfedge_Mesh& mesh, HalfedgeOpt opt);
    
    static void set_he_select(unsigned int id);
    static void set_he_select(Halfedge_Mesh::ElementRef elem);
    static void set_he_hover(Vec2 mouse);
    static unsigned int get_he_select();
    static std::optional<Halfedge_Mesh::ElementRef> he_selected();
    
    static void begin_transform(Gui::Action action);
    static bool apply_transform(Gui::Action action, Pose delta);

    static void mesh(const GL::Mesh& mesh, MeshOpt opt);
    static void lines(const GL::Lines& lines, Mat4 viewproj, float alpha);
    static void outline(Mat4 viewproj, Mat4 view, Scene_Object& obj);

private:
    void build_halfedge(Halfedge_Mesh& mesh);

    Renderer(Vec2 dim);
    ~Renderer();
    static inline Renderer* data = nullptr;

    struct transform_data {
        std::vector<Vec3> verts;
        Vec3 center;
    };

    int samples;
    Vec2 window_dim;
    GLubyte* id_buffer;
    transform_data first_t;
	GL::Framebuffer framebuffer, id_resolve;
    GL::Shader mesh_shader, line_shader, inst_shader; 
    GL::Instances spheres, cylinders, arrows;
    GL::Mesh face_mesh;
    
    Mat4 _proj;
    Halfedge_Mesh* loaded_mesh = nullptr;
    
    // This all needs to be updated when the mesh connectivity changes
    unsigned int selected_compo = -1, hover_compo = -1;

    // TODO(max): this is a kind of bad design and would be un-necessary if we used
    // a halfedge implementation with contiguous iterators. For now this map must
    // be updated (along with the instance data) by build_halfedge whenever
    // the mesh changes its connectivity. Note that build_halfedge also
    // re-indexes the mesh elements in the provided half-edge mesh.
    std::map<unsigned int, Halfedge_Mesh::ElementRef> idx_to_elm;
};
