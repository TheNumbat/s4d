
#include "scene.h"
#include "proc_objects.h"

#include <imgui/imgui.h>

Scene::Scene(Vec2 window_dim) :
    mesh_shader("mesh.vert", "mesh.frag"),
    line_shader("line.vert", "line.frag"),
    window_dim(window_dim),
    camera(window_dim),
    baseplane(0.5f) {

    create_baseplane();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonOffset(1.0f, 1.0f);

    // use reversed depth
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glClearDepthf(0.0f);
}

Scene::~Scene() {

}

void Scene::create_baseplane() {

    const int R = 25;
    for(int i = -R; i <= R; i++) {
        if(i == 0) {
            baseplane.add({-R, 0, i}, {R, 0, i}, {0.6f, 0.1f, 0.1f});
            baseplane.add({i, 0, -R}, {i, 0, R}, {0.1f, 0.6f, 0.1f});
            continue;
        }
        baseplane.add({i, 0, -R}, {i, 0, R}, {0.5f, 0.5f, 0.5f});
        baseplane.add({-R, 0, i}, {R, 0, i}, {0.5f, 0.5f, 0.5f});
    }
}

void Scene::reload_shaders() {
    mesh_shader.reload();
    line_shader.reload();
}

void Scene::render() {

	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Mat4 proj = camera.proj(), view = camera.view();;
    Mat4 viewproj = proj * view;

    {
        glEnable(GL_POLYGON_OFFSET_FILL);

        mesh_shader.bind();
        glUniformMatrix4fv(mesh_shader.uniform("proj"), 1, GL_FALSE, proj.data);

        for(auto& obj : objs) {
            obj.second.render(view, mesh_shader);
        }
        
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    {
        line_shader.bind();
        glUniformMatrix4fv(line_shader.uniform("viewproj"), 1, GL_FALSE, viewproj.data);
        baseplane.render();
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
