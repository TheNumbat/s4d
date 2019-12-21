
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
	baseplane(1.0f) {

	state.window_dim = window_dim;
	id_buffer = new float[(int)state.window_dim.x * (int)state.window_dim.y * 3];
	GL::global_params();
	create_baseplane();

	state.x_trans = Scene_Object((Scene_Object::ID)Gui::Basic::x_trans, Pose::rotate({0.0f, 0.0f, -90.0f}), Util::arrow(), {0.6f, 0.1f, 0.1f});
	state.y_trans = Scene_Object((Scene_Object::ID)Gui::Basic::y_trans, {}, Util::arrow(), {0.1f, 0.6f, 0.1f});
	state.z_trans = Scene_Object((Scene_Object::ID)Gui::Basic::z_trans, Pose::rotate({90.0f, 0.0f, 0.0f}), Util::arrow(), {0.1f, 0.1f, 0.6f});
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
	state.settings_open = true;
}

void Scene::reload_shaders() {
	mesh_shader.reload();
	line_shader.reload();
}

Scene_Object::ID Scene::read_id(Vec2 pos) {
	
	int x = (int)pos.x;
	int y = (int)(state.window_dim.y - pos.y - 1);
	int idx = y * (int)state.window_dim.x * 3 + x * 3;
	
	int a = (int)(id_buffer[idx] * 255.0f);
	int b = (int)(id_buffer[idx + 1] * 255.0f);
	int c = (int)(id_buffer[idx + 2] * 255.0f);

	return a | b << 8 | c << 16;
}

void Scene::render_widgets(const Scene_Object& obj) {

	framebuffer.clear_d();

	mesh_shader.bind();
	obj.render(view, mesh_shader, false, true);
	
	Mat4 mvp = viewproj * obj.pose.transform();
	Vec2 min, max;
	obj.bbox().project(mvp, min, max);

	GL::Effects::outline(framebuffer, framebuffer, Scene_Object::outline_color, min, max);

	framebuffer.clear_d();

	mesh_shader.bind();
	if(state.action == Gui::Action::move) {
		
		// TODO(max): this only scales correctly given a constant object position...
		float scl = (camera.pos() - obj.pose.pos).norm() / 5.0f;
		Vec3 scale = Vec3(scl, scl, scl);

		state.x_trans.pose.scl = scale;
		state.x_trans.pose.pos = obj.pose.pos + Vec3(0.15f * scl, 0.0f, 0.0f);
		state.x_trans.render(view, mesh_shader, true);

		state.y_trans.pose.scl = scale;
		state.y_trans.pose.pos = obj.pose.pos + Vec3(0.0f, 0.15f * scl, 0.0f);
		state.y_trans.render(view, mesh_shader, true);

		state.z_trans.pose.scl = scale;
		state.z_trans.pose.pos = obj.pose.pos + Vec3(0.0f, 0.0f, 0.15f * scl);
		state.z_trans.render(view, mesh_shader, true);
	}
}

void Scene::render() {

	proj = camera.proj(), view = camera.view();
	viewproj = proj * view;

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

	auto selected = objs.find(state.id);
	if(selected != objs.end()) {
		render_widgets(selected->second);
	}

	framebuffer.blit_to(1, id_resolve, false);
	id_resolve.read(0, id_buffer);

	framebuffer.blit_to_screen(0, state.window_dim);
}

void Scene::mouse_pos(Vec2 dmouse) {

	
}

bool Scene::select(Vec2 mouse) {
	
	Scene_Object::ID clicked = read_id(mouse);
	state.selecting = true;

	switch(clicked) {
	case (Scene_Object::ID)Gui::Basic::x_trans: {
		state.action = Gui::Action::move;
		state.axis = Gui::Axis::X;
	} break;
	case (Scene_Object::ID)Gui::Basic::y_trans: {
		state.action = Gui::Action::move;
		state.axis = Gui::Axis::Y;
	} break;
	case (Scene_Object::ID)Gui::Basic::z_trans: {
		state.action = Gui::Action::move;
		state.axis = Gui::Axis::Z;
	} break;
	case (Scene_Object::ID)Gui::Basic::x_rot: {
		state.action = Gui::Action::rotate;
		state.axis = Gui::Axis::X;
	} break;
	case (Scene_Object::ID)Gui::Basic::y_rot: {
		state.action = Gui::Action::rotate;
		state.axis = Gui::Axis::Y;
	} break;
	case (Scene_Object::ID)Gui::Basic::z_rot: {
		state.action = Gui::Action::rotate;
		state.axis = Gui::Axis::Z;
	} break;
	case (Scene_Object::ID)Gui::Basic::x_scale: {
		state.action = Gui::Action::scale;
		state.axis = Gui::Axis::X;
	} break;
	case (Scene_Object::ID)Gui::Basic::y_scale: {
		state.action = Gui::Action::scale;
		state.axis = Gui::Axis::Y;
	} break;
	case (Scene_Object::ID)Gui::Basic::z_scale: {
		state.action = Gui::Action::scale;
		state.axis = Gui::Axis::Z;
	} break;
	default: {
		state.selecting = false;
		state.id = clicked; 
	} break;
	}

	return state.selecting;
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

	state.window_dim = new_dim;

	delete[] id_buffer;
	id_buffer = new float[(int)state.window_dim.x * (int)state.window_dim.y * 3];

	camera.set_ar(state.window_dim);
	framebuffer.resize(state.window_dim, samples);
	id_resolve.resize(state.window_dim);
	GL::viewport(state.window_dim);
}

void Scene::gui() {

	const ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

	ImGui::SetNextWindowPos({0.0, 18.0});
	ImGui::SetNextWindowSize({state.window_dim.x / 5.0f, state.window_dim.y});

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
		
		bool selected = entry.first == state.id;
		ImGui::SameLine();
		if(ImGui::Checkbox("##selected", &selected)) {
			if(selected) state.id = entry.first;
			else state.id = 0;
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

	if(state.settings_open) {
		ImGui::Begin("Display Settings", &state.settings_open);
		
		ImGui::InputInt("Multisampling", &samples);
		if(samples < 1) samples = 1;
		if(samples > 16) samples = 16;

		if(ImGui::Button("Apply")) {
			framebuffer.resize(state.window_dim, samples);
		}
		ImGui::End();
	}
}
