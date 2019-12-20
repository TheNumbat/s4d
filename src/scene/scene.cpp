
#include "scene.h"
#include "util.h"
#include "../app.h"

#include <nfd/nfd.h>
#include <imgui/imgui.h>

Scene::Scene(Vec2 window_dim, App& app) :
	app(app),
	camera(window_dim),
	mesh_shader("mesh.vert", "mesh.frag"),
	line_shader("line.vert", "line.frag"),
	framebuffer(2, window_dim, default_samples),
	id_resolve(1, window_dim, 1, false),
	baseplane(1.0f),
	window_dim(window_dim) {

	id_buffer = new float[(int)window_dim.x * (int)window_dim.y * 3];
	GL::global_params();
	create_baseplane();

	x_trans = Scene_Object((Scene_Object::ID)Base_Objs::x_trans, Pose::rotate({0.0f, 0.0f, -90.0f}), Util::arrow(), {0.6f, 0.1f, 0.1f});
	y_trans = Scene_Object((Scene_Object::ID)Base_Objs::y_trans, {}, Util::arrow(), {0.1f, 0.6f, 0.1f});
	z_trans = Scene_Object((Scene_Object::ID)Base_Objs::z_trans, Pose::rotate({90.0f, 0.0f, 0.0f}), Util::arrow(), {0.1f, 0.1f, 0.6f});
}

Scene::~Scene() {
	delete[] id_buffer;
}

void Scene::create_baseplane() {

	const int R = 25;
	for(int i = -R; i <= R; i++) {
		if(i == 0) {
			baseplane.add({-R, 0, i}, {R, 0, i}, {0.6f, 0.1f, 0.1f});
			baseplane.add({i, 0, -R}, {i, 0, R}, {0.1f, 0.1f, 0.6f});
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

void Scene::render_widgets(const Scene_Object& obj) {

	mesh_shader.bind();
	Mat4 view = camera.view();

	framebuffer.clear_d();

	obj.render(view, mesh_shader, false, true);
	GL::Effects::outline(framebuffer, framebuffer, Scene_Object::outline_color);

	framebuffer.clear_d();

	mesh_shader.bind();
	if(select_type == Select_Type::move) {
		
		// TODO(max): this only scales correctly given a constant object position...
		float scl = (camera.pos() - obj.pose.pos).norm() / 5.0f;
		Vec3 scale = Vec3(scl, scl, scl);

		x_trans.pose.scl = scale;
		x_trans.pose.pos = obj.pose.pos + Vec3(0.15f * scl, 0.0f, 0.0f);
		x_trans.render(view, mesh_shader, true);

		y_trans.pose.scl = scale;
		y_trans.pose.pos = obj.pose.pos + Vec3(0.0f, 0.15f * scl, 0.0f);
		y_trans.render(view, mesh_shader, true);

		z_trans.pose.scl = scale;
		z_trans.pose.pos = obj.pose.pos + Vec3(0.0f, 0.0f, 0.15f * scl);
		z_trans.render(view, mesh_shader, true);
	}
}

void Scene::render() {

	Mat4 proj = camera.proj(), view = camera.view();
	Mat4 viewproj = proj * view;

	framebuffer.clear(0, {0.4f, 0.4f, 0.4f, 1.0f});
	framebuffer.clear(1, {0.0f, 0.0f, 0.0f, 1.0f});
	framebuffer.clear_d();
	framebuffer.bind();
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

	auto selected = objs.find(selected_id);
	if(selected != objs.end()) {
		render_widgets(selected->second);
	}

	framebuffer.blit_to(1, id_resolve, false);
	id_resolve.read(0, id_buffer);

	framebuffer.blit_to_screen(0, window_dim);
}

void Scene::select(Vec2 mouse) {
	
	Scene_Object::ID clicked = read_id(mouse);

	if(clicked == 0 || clicked >= (Scene_Object::ID)Base_Objs::count) {
		selected_id = clicked;
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

	delete[] id_buffer;
	id_buffer = new float[(int)window_dim.x * (int)window_dim.y * 3];

	camera.set_ar(window_dim);
	framebuffer.resize(window_dim, samples);
	id_resolve.resize(window_dim);
	GL::viewport(window_dim);
}

void Scene::gui() {

	const ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

	ImGui::SetNextWindowPos({0.0, 18.0});
	ImGui::SetNextWindowSize({window_dim.x / 5.0f, window_dim.y});

	ImGui::Begin("Objects", nullptr, flags);

	if(ImGui::Button("Create Object")) {
		ImGui::OpenPopup("Type");
	}
	if(ImGui::BeginPopup("Type")) {
		
		if(ImGui::Button("Cube")) {
			add_object(Scene_Object(next_id++, {}, Util::cube_mesh(1.0f)));
			ImGui::CloseCurrentPopup();
		}

		if(ImGui::Button("Cylinder")) {
			add_object(Scene_Object(next_id++, {}, Util::cyl_mesh(1.0f, 1.0f)));
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if(ImGui::Button("Load Object")) {
		char* path = nullptr;
		NFD_OpenDialog("obj", nullptr, &path);
		
		if(path) {
			GL::Mesh new_mesh;
			std::string error = Util::obj_mesh(std::string(path), new_mesh);
			if(error.empty()) {
				add_object(Scene_Object(next_id++, {}, std::move(new_mesh)));
			} else {
				app.gui_error(error);
			}
			free(path);
		}
	}

	if(objs.size() > 0)
		ImGui::Separator();

	int i = 0;
	Scene_Object::ID to_delete = 0;

	for(auto& entry : objs) {

		ImGui::PushID(entry.first);

		Scene_Object& obj = entry.second;
		std::string& name = obj.opt.name;
		ImGui::InputText("##name", name.data(), name.capacity());
		
		bool selected = entry.first == selected_id;
		ImGui::SameLine();
		if(ImGui::Checkbox("##selected", &selected)) {
			if(selected) selected_id = entry.first;
			else selected_id = 0;
		}

		ImGui::Checkbox("Wireframe", &obj.opt.wireframe);
		ImGui::DragFloat3("Position", obj.pose.pos.data, 0.1f);
		ImGui::DragFloat3("Rotation", obj.pose.euler.data);
		ImGui::DragFloat3("Scale", obj.pose.scl.data, 0.01f);
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
