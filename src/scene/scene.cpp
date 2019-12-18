
#include "scene.h"
#include "proc_objects.h"

#include <imgui/imgui.h>

Scene::Scene(Vec2 window_dim) :
	mesh_shader("mesh.vert", "mesh.frag"),
	line_shader("line.vert", "line.frag"),
	window_dim(window_dim),
	camera(window_dim),
	framebuffer(2, window_dim, default_samples),
	id_resolve(1, window_dim),
	baseplane(1.0f) {

	id_buffer = new float[(int)window_dim.x * (int)window_dim.y * 3];
	GL::global_params();
	create_baseplane();
}

Scene::~Scene() {
	delete[] id_buffer;
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

void Scene::show_settings() {
	settings_open = true;
}

void Scene::reload_shaders() {
	mesh_shader.reload();
	line_shader.reload();
}

Scene_Object::ID Scene::read_id(Vec2 pos) {
	
	int x = (int)pos.x;
	int y = (int)(window_dim.y - pos.y - 1);
	int idx = y * (int)window_dim.x * 3 + x * 3;
	
	int a = (int)(id_buffer[idx] * 255.0f);
	int b = (int)(id_buffer[idx + 1] * 255.0f);
	int c = (int)(id_buffer[idx + 2] * 255.0f);

	return a | b << 8 | c << 16;
}

void Scene::render() {

	Mat4 proj = camera.proj(), view = camera.view();
	Mat4 viewproj = proj * view;

	framebuffer.clear(0, {0.4f, 0.4f, 0.4f, 1.0f});
	framebuffer.clear(1, {0.0f, 0.0f, 0.0f, 1.0f});
	framebuffer.clear_ds();
	framebuffer.bind();
	{
		mesh_shader.bind();
		mesh_shader.uniform("proj", proj);

		for(auto& obj : objs) {
			obj.second.render(view, obj.first == selected_id, mesh_shader);
		}
	}
	framebuffer.blit_to(1, id_resolve, false);
	id_resolve.read(0, id_buffer);

	framebuffer.bind();
	{
		line_shader.bind();
		line_shader.uniform("viewproj", viewproj);
		baseplane.render();
	}

	framebuffer.blit_to_screen(0, window_dim);
}

void Scene::select(Vec2 mouse) {
	selected_id = read_id(mouse);
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

	delete[] id_buffer;
	id_buffer = new float[(int)window_dim.x * (int)window_dim.y * 3];

	camera.set_ar(window_dim);
	framebuffer.resize(window_dim, samples);
	GL::viewport(window_dim);
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

		std::string& name = entry.second.opt.name;
		ImGui::InputText("##name", name.data(), name.capacity());
		
		bool selected = entry.first == selected_id;
		ImGui::SameLine();
		if(ImGui::Checkbox("##selected", &selected)) {
			if(selected) selected_id = entry.first;
			else selected_id = 0;
		}

		ImGui::Checkbox("Wireframe", &entry.second.opt.wireframe);
		if(ImGui::SmallButton("Delete")) {
			to_delete = entry.first;
		}
		if(i++ != objs.size() - 1) ImGui::Separator();

		ImGui::PopID();
	}

	if(to_delete) objs.erase(to_delete);

	ImGui::End();

	if(settings_open) {
		ImGui::Begin("Display Settings", &settings_open);
		
		ImGui::InputInt("Multisampling", &samples);
		if(samples < 1) samples = 1;
		if(samples > 16) samples = 16;

		if(ImGui::Button("Apply")) {
			framebuffer.resize(window_dim, samples);
		}
		ImGui::End();
	}
}
