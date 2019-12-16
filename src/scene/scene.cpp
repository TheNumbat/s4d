
#include "scene.h"
#include "proc_objects.h"

#include <imgui/imgui.h>

Scene::Scene(Vec2 window_dim) :
    mesh_shader("mesh.vert", "mesh.frag"),
    window_dim(window_dim),
    camera(window_dim) {
}

Scene::~Scene() {

}

void Scene::render() {

    glEnable(GL_DEPTH_TEST);

    mesh_shader.bind();

    Mat4 proj_view = camera.proj() * camera.view();
    glUniformMatrix4fv(mesh_shader.uniform("proj_view"), 1, GL_FALSE, proj_view.data);

    for(auto& obj : objs) {
        obj.second.render(mesh_shader);
    }
}

void Scene::add_object(Scene_Object&& obj) {
    objs.emplace(std::make_pair(obj.id(), std::move(obj)));
}

void Scene::camera_orbit(Vec2 dmouse) {
    camera.mouse_orbit(dmouse);
}

void Scene::camera_move(Vec2 dmouse) {
    camera.mouse_move(dmouse);
}

void Scene::camera_radius(float dmouse) {
    camera.mouse_radius(dmouse);
}

void Scene::apply_window_dim(Vec2 new_dim) {

    window_dim = new_dim;
    camera.set_ar(window_dim);
    glViewport(0, 0, (GLsizei)window_dim.x, (GLsizei)window_dim.y);
}

void Scene::gui() {

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

    ImGui::SetNextWindowPos({0.0, 18.0});
    ImGui::SetNextWindowSize({window_dim.x / 5.0f, window_dim.y});

    ImGui::Begin("Objects", nullptr, flags);

    if(ImGui::Button("Create Object")) {
        GL_Mesh cube = Proc_Objects::cube(1.0f);
        add_object(Scene_Object(next_id++, Mat4::I, std::move(cube)));
    }
    if(ImGui::Button("Load Object")) {

    }

    if(objs.size() > 0)
        ImGui::Separator();

    int i = 0;
    Scene_Object::ID to_delete = 0;

    for(auto& entry : objs) {

        ImGui::PushID(entry.first);

        if(entry.first == selected_id) {
            ImGui::Text("[Object %d]", entry.first);
        } else {
            ImGui::Text("Object %d", entry.first);
        }

        if(ImGui::SmallButton("Select")) {
            selected_id = entry.first;
        }
        if(ImGui::SmallButton("Delete")) {
            to_delete = entry.first;
        }
        if(i++ != objs.size() - 1) ImGui::Separator();

        ImGui::PopID();
    }

    if(to_delete) objs.erase(to_delete);

    ImGui::End();
}
