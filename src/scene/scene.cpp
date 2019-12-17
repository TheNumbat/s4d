
#include "scene.h"
#include "proc_objects.h"

#include <imgui/imgui.h>

Scene::Scene(Vec2 window_dim) :
	mesh_shader("mesh.vert", "mesh.frag"),
	line_shader("line.vert", "line.frag"),
	window_dim(window_dim),
	camera(window_dim),
	framebuffer(1, window_dim, 4),
	baseplane(1.0f) {

	GL::global_params();
	create_baseplane();
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

	GL::clear_screen({});

	Mat4 proj = camera.proj(), view = camera.view();
	Mat4 viewproj = proj * view;

	framebuffer.bind();
	framebuffer.clear({0.4f, 0.4f, 0.4f, 1.0f});
	{
		mesh_shader.bind();
		mesh_shader.uniform("proj", proj);

		for(auto& obj : objs) {
			obj.second.render(view, mesh_shader);
		}
	}

	{
		line_shader.bind();
		line_shader.uniform("viewproj", viewproj);
		baseplane.render();
	}

	framebuffer.blit_to_screen(0, window_dim);
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
	GL::viewport(new_dim);
}

void Scene::gui() {

	const ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

	ImGui::SetNextWindowPos({0.0, 18.0});
	ImGui::SetNextWindowSize({window_dim.x / 5.0f, window_dim.y});

	ImGui::Begin("Objects", nullptr, flags);

	if(ImGui::Button("Create Object")) {
		GL::Mesh cube = Proc_Objects::cube(1.0f);
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

		ImGui::InputText("##name", entry.second.name.data(), entry.second.name.capacity());
		
		bool selected = entry.first == selected_id;
		ImGui::SameLine();
		if(ImGui::Checkbox("##selected", &selected)) {
			if(selected) selected_id = entry.first;
			else selected_id = 0;
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
