
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
	id_buffer = new unsigned char[(int)state.window_dim.x * (int)state.window_dim.y * 4];
	GL::global_params();
	create_baseplane();

	state.x_trans = Scene_Object((Scene_Object::ID)Gui::Basic::x_trans, Pose::rotated({0.0f, 0.0f, -90.0f}), Util::arrow_mesh(), Gui::red);
	state.y_trans = Scene_Object((Scene_Object::ID)Gui::Basic::y_trans, {}, Util::arrow_mesh(), Gui::green);
	state.z_trans = Scene_Object((Scene_Object::ID)Gui::Basic::z_trans, Pose::rotated({90.0f, 0.0f, 0.0f}), Util::arrow_mesh(), Gui::blue);

	state.x_rot = Scene_Object((Scene_Object::ID)Gui::Basic::x_rot, Pose::rotated({0.0f, 0.0f, -90.0f}), Util::torus_mesh(0.975f, 1.0f), Gui::red);
	state.y_rot = Scene_Object((Scene_Object::ID)Gui::Basic::y_rot, {}, Util::torus_mesh(0.975f, 1.0f), Gui::green);
	state.z_rot = Scene_Object((Scene_Object::ID)Gui::Basic::z_rot, Pose::rotated({90.0f, 0.0f, 0.0f}), Util::torus_mesh(0.975f, 1.0f), Gui::blue);

	state.x_scale = Scene_Object((Scene_Object::ID)Gui::Basic::x_scale, Pose::rotated({0.0f, 0.0f, -90.0f}), Util::scale_mesh(), Gui::red);
	state.y_scale = Scene_Object((Scene_Object::ID)Gui::Basic::y_scale, {}, Util::scale_mesh(), Gui::green);
	state.z_scale = Scene_Object((Scene_Object::ID)Gui::Basic::z_scale, Pose::rotated({90.0f, 0.0f, 0.0f}), Util::scale_mesh(), Gui::blue);
}

Scene::~Scene() {
	delete[] id_buffer;
}

void Scene::create_baseplane() {

	const int R = 25;
	for(int i = -R; i <= R; i++) {
		if(i == 0) {
			baseplane.add({-R, 0, i}, {R, 0, i}, Gui::red);
			baseplane.add({i, 0, -R}, {i, 0, R}, Gui::blue);
			continue;
		}
		baseplane.add({i, 0, -R}, {i, 0, R}, Gui::baseplane);
		baseplane.add({-R, 0, i}, {R, 0, i}, Gui::baseplane);
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
	int idx = y * (int)state.window_dim.x * 4 + x * 4;
	
	int a = id_buffer[idx];
	int b = id_buffer[idx + 1];
	int c = id_buffer[idx + 2];

	return a | b << 8 | c << 16;
}

void Scene::render_selected(Scene_Object& obj) {

	Vec3 prev_scale = obj.pose.scale;
	Vec3 prev_rot = obj.pose.euler;
	if(state.dragging) {
		if(state.action == Gui::Action::scale) obj.pose.scale *= model_end(obj);
		if(state.action == Gui::Action::rotate) obj.pose.euler += model_end(obj);
	}

	mesh_shader.bind();
	obj.render(view, mesh_shader);

	framebuffer.clear_d();
	obj.render(view, mesh_shader, false, true);
	
	// NOTE(max): NVIDIA driver synchronization bug
	GL::flush();

	Vec2 min, max;
	obj.bbox().screen_rect(viewproj, min, max);

	GL::Effects::outline(framebuffer, framebuffer, Gui::outline, 
						 min - Vec2(3.0f / state.window_dim.x), 
						 max + Vec2(3.0f / state.window_dim.y));
	
	framebuffer.clear_d();

	// TODO(max): this only scales correctly given a constant object position...
	float scl = (camera.pos() - obj.pose.pos).norm() / 5.0f;
	Vec3 scale = Vec3(scl, scl, scl);

	mesh_shader.bind();
	if(state.action == Gui::Action::move) {

		state.x_trans.pose.scale = scale;
		state.x_trans.pose.pos = obj.pose.pos + Vec3(0.15f * scl, 0.0f, 0.0f);
		state.x_trans.render(view, mesh_shader, true);

		state.y_trans.pose.scale = scale;
		state.y_trans.pose.pos = obj.pose.pos + Vec3(0.0f, 0.15f * scl, 0.0f);
		state.y_trans.render(view, mesh_shader, true);

		state.z_trans.pose.scale = scale;
		state.z_trans.pose.pos = obj.pose.pos + Vec3(0.0f, 0.0f, 0.15f * scl);
		state.z_trans.render(view, mesh_shader, true);
	
	} else if(state.action == Gui::Action::rotate) {

		state.x_rot.pose.scale = scale;
		state.x_rot.pose.pos = obj.pose.pos;
		state.x_rot.render(view, mesh_shader, true);

		state.y_rot.pose.scale = scale;
		state.y_rot.pose.pos = obj.pose.pos;
		state.y_rot.render(view, mesh_shader, true);

		state.z_rot.pose.scale = scale;
		state.z_rot.pose.pos = obj.pose.pos;
		state.z_rot.render(view, mesh_shader, true);

	} else if(state.action == Gui::Action::scale) {

		state.x_scale.pose.scale = scale;
		state.x_scale.pose.pos = obj.pose.pos + Vec3(0.15f * scl, 0.0f, 0.0f);
		state.x_scale.render(view, mesh_shader, true);

		state.y_scale.pose.scale = scale;
		state.y_scale.pose.pos = obj.pose.pos + Vec3(0.0f, 0.15f * scl, 0.0f);
		state.y_scale.render(view, mesh_shader, true);

		state.z_scale.pose.scale = scale;
		state.z_scale.pose.pos = obj.pose.pos + Vec3(0.0f, 0.0f, 0.15f * scl);
		state.z_scale.render(view, mesh_shader, true);
	}

	obj.pose.scale = prev_scale;
	obj.pose.euler = prev_rot;
}

void Scene::render() {

	proj = camera.proj(), view = camera.view();
	viewproj = proj * view;
	iviewproj = Mat4::inverse(viewproj);

	framebuffer.clear(0, Vec4(Gui::background, 1.0f));
	framebuffer.clear(1, {0.0f, 0.0f, 0.0f, 1.0f});
	framebuffer.clear_d();
	framebuffer.bind();
	{
		mesh_shader.bind();
		mesh_shader.uniform("proj", proj);

		for(auto& obj : objs) {
			if(obj.first != state.id) 
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
		render_selected(selected->second);
	}

	framebuffer.blit_to(1, id_resolve, false);
	id_resolve.read(0, id_buffer);

	framebuffer.blit_to_screen(0, state.window_dim);
}

Vec3 Scene::model_end(Scene_Object& obj) {
	Vec3 axis = state.action == Gui::Action::rotate ? Vec3(0.0f) : Vec3(1.0f);
	axis[(int)state.axis] = state.end;
	// TODO(max): this
	return axis;
}

Vec3 Scene::screen_to_world(Vec2 mouse) {

	Vec2 t(2.0f * mouse.x / state.window_dim.x - 1.0f, 
		   1.0f - 2.0f * mouse.y / state.window_dim.y);
	return (iviewproj * Vec3(t.x, t.y, 0.1f));
}

bool Scene::screen_to_axis(Scene_Object& obj, Vec2 mouse, Vec3& hit) {
	
	Vec3 axis; axis[(int)state.axis] = 1.0f;
	Vec3 dir = (screen_to_world(mouse) - camera.pos()).unit();
	
	Line select(camera.pos(), dir);
	Line target(obj.pose.pos, axis);
	Plane plane(obj.pose.pos, -camera.front().unit());

	bool ret = plane.hit(select, hit);
	hit = target.closest(hit);
	return ret && hit.valid();
 }

bool Scene::screen_to_plane(Scene_Object& obj, Vec2 mouse, Vec3& hit) {
	Vec3 world = screen_to_world(mouse);
	Vec3 cam = camera.pos();
	Line look(cam, (world - cam).unit());
	Vec3 axis; axis[(int)state.axis] = 1.0f;
	Plane p(obj.pose.pos, axis);
	return p.hit(look, hit);
}

void Scene::end_drag(Vec2 mouse) {

	if(!state.dragging) return;

	Scene_Object& obj = objs[state.id];

	if(state.action == Gui::Action::scale) {
		obj.pose.scale *= model_end(obj);
	} else if(state.action == Gui::Action::rotate) {
		obj.pose.euler += model_end(obj); 
	}

	state.dragging = false;
}

void Scene::drag(Vec2 mouse) {

	if(!state.dragging) return;
	
	Scene_Object& obj = objs[state.id];
	Vec3 pos = obj.pose.pos;
	
	if(state.action == Gui::Action::rotate) {
		Vec3 hit;
		if(!screen_to_plane(obj, mouse, hit)) return;
		Vec3 ang = (hit - pos).unit();
		float sgn = sign(cross(state.start, ang)[(int)state.axis]);
		state.end = sgn * Degrees(std::acos(dot(state.start, ang)));
		return;
	}

	Vec3 hit;
	if(!screen_to_axis(obj, mouse, hit)) return;

	if(state.action == Gui::Action::move) {
		obj.pose.pos = hit - state.start;
	} else if(state.action == Gui::Action::scale) {
		state.end = (hit - pos).norm() / state.start.norm();
	}
}

bool Scene::select(Vec2 mouse) {
	
	Scene_Object::ID clicked = read_id(mouse);
	state.dragging = true;

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
		state.dragging = false;
		state.id = clicked; 
	} break;
	}

	if(state.dragging) {
		Scene_Object& obj = objs[state.id];
		Vec3 hit;
		if(state.action == Gui::Action::rotate && screen_to_plane(obj, mouse, hit)) {
			state.start = (hit - obj.pose.pos).unit();
			state.end = 0.0f;
		} else if(screen_to_axis(obj, mouse, hit)) {
			state.start = hit - obj.pose.pos;
			state.end = 1.0f;
		}
	}
	return state.dragging;
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
	id_buffer = new unsigned char[(int)state.window_dim.x * (int)state.window_dim.y * 4]();

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

		if(ImGui::Button("Torus")) {
			add_object(Scene_Object(next_id++, {}, Util::torus_mesh(0.8f, 1.0f)));
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

		if(ImGui::DragFloat3("Position", obj.pose.pos.data, 0.1f) && selected)
			state.action = Gui::Action::move;
		
		if(state.dragging && state.action == Gui::Action::rotate) {
			Vec3 fake_rot = obj.pose.euler;
			fake_rot[(int)state.axis] += state.end;
			ImGui::DragFloat3("Rotation", fake_rot.range(0.0f, 360.0f).data);
		} else {
			if(ImGui::DragFloat3("Rotation", obj.pose.euler.data) && selected)
				state.action = Gui::Action::rotate;
		}
		
		obj.pose.clamp_euler();

		if(state.dragging && state.action == Gui::Action::scale) {
			Vec3 fake_scale = obj.pose.scale * state.end;
			ImGui::DragFloat3("Scale", fake_scale.data, 0.01f);
		} else {
			if(ImGui::DragFloat3("Scale", obj.pose.scale.data, 0.01f) && selected)
				state.action = Gui::Action::scale;
		}

		if(selected) {
			if(ImGui::SmallButton("Move"))
				state.action = Gui::Action::move;
			ImGui::SameLine();
			if(ImGui::SmallButton("Rotate"))
				state.action = Gui::Action::rotate;
			if(ImGui::SmallButton("Scale"))
				state.action = Gui::Action::scale;
			ImGui::SameLine();
			if(ImGui::SmallButton("Delete"))
				to_delete = entry.first;
		}

		if(i++ != objs.size() - 1) ImGui::Separator();

		ImGui::PopID();
	}

	if(to_delete) objs.erase(to_delete);

	ImGui::End();

	if(state.settings_open) {
		ImGui::Begin("Display Settings", &state.settings_open, 
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
		
		ImGui::InputInt("Multisampling", &samples);
		if(samples < 1) samples = 1;
		if(samples > 16) samples = 16;

		if(ImGui::Button("Apply")) {
			framebuffer.resize(state.window_dim, samples);
		}

		ImGui::Separator();
		ImGui::Text("%s", glGetString(GL_VERSION));
		ImGui::Text("%s", glGetString(GL_RENDERER));
		ImGui::Text("%s", glGetString(GL_SHADING_LANGUAGE_VERSION));

		ImGui::End();
	}
}
