
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
}

Scene::Gui::Gui() {
	x_trans = Scene_Object((Scene_Object::ID)Gui::Basic::x_trans, Pose::rotated({0.0f, 0.0f, -90.0f}), Util::arrow_mesh(), Gui::red);
	y_trans = Scene_Object((Scene_Object::ID)Gui::Basic::y_trans, {}, Util::arrow_mesh(), Gui::green);
	z_trans = Scene_Object((Scene_Object::ID)Gui::Basic::z_trans, Pose::rotated({90.0f, 0.0f, 0.0f}), Util::arrow_mesh(), Gui::blue);

	xy_trans = Scene_Object((Scene_Object::ID)Gui::Basic::xy_trans, Pose::rotated({-90.0f, 0.0f, 0.0f}), Util::square_mesh(0.1f), Gui::blue);
	yz_trans = Scene_Object((Scene_Object::ID)Gui::Basic::yz_trans, Pose::rotated({0.0f, 0.0f, -90.0f}), Util::square_mesh(0.1f), Gui::red);
	xz_trans = Scene_Object((Scene_Object::ID)Gui::Basic::xz_trans, {}, Util::square_mesh(0.1f), Gui::green);

	x_rot = Scene_Object((Scene_Object::ID)Gui::Basic::x_rot, Pose::rotated({0.0f, 0.0f, -90.0f}), Util::torus_mesh(0.975f, 1.0f), Gui::red);
	y_rot = Scene_Object((Scene_Object::ID)Gui::Basic::y_rot, {}, Util::torus_mesh(0.975f, 1.0f), Gui::green);
	z_rot = Scene_Object((Scene_Object::ID)Gui::Basic::z_rot, Pose::rotated({90.0f, 0.0f, 0.0f}), Util::torus_mesh(0.975f, 1.0f), Gui::blue);

	x_scale = Scene_Object((Scene_Object::ID)Gui::Basic::x_scale, Pose::rotated({0.0f, 0.0f, -90.0f}), Util::scale_mesh(), Gui::red);
	y_scale = Scene_Object((Scene_Object::ID)Gui::Basic::y_scale, {}, Util::scale_mesh(), Gui::green);
	z_scale = Scene_Object((Scene_Object::ID)Gui::Basic::z_scale, Pose::rotated({90.0f, 0.0f, 0.0f}), Util::scale_mesh(), Gui::blue);
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

void Scene::Gui::render_widgets(Mat4 view, GL::Shader& shader, Vec3 pos, float scl) {

	Vec3 scale(scl);

	shader.bind();
	if(action == Gui::Action::move) {

		x_trans.pose.scale = scale;
		x_trans.pose.pos = pos + Vec3(0.15f * scl, 0.0f, 0.0f);
		x_trans.render(view, shader, true);

		y_trans.pose.scale = scale;
		y_trans.pose.pos = pos + Vec3(0.0f, 0.15f * scl, 0.0f);
		y_trans.render(view, shader, true);

		z_trans.pose.scale = scale;
		z_trans.pose.pos = pos + Vec3(0.0f, 0.0f, 0.15f * scl);
		z_trans.render(view, shader, true);

		GL::disable(GL::Opt::culling);
		xy_trans.pose.scale = scale;
		xy_trans.pose.pos = pos + Vec3(0.45f * scl, 0.45f * scl, 0.0f);
		xy_trans.render(view, shader, true);

		yz_trans.pose.scale = scale;
		yz_trans.pose.pos = pos + Vec3(0.0f, 0.45f * scl, 0.45f * scl);
		yz_trans.render(view, shader, true);

		xz_trans.pose.scale = scale;
		xz_trans.pose.pos = pos + Vec3(0.45f * scl, 0.0f, 0.45f * scl);
		xz_trans.render(view, shader, true);
		GL::enable(GL::Opt::culling);
	
	} else if(action == Gui::Action::rotate) {

		x_rot.pose.scale = scale;
		x_rot.pose.pos = pos;
		x_rot.render(view, shader, true);

		y_rot.pose.scale = scale;
		y_rot.pose.pos = pos;
		y_rot.render(view, shader, true);

		z_rot.pose.scale = scale;
		z_rot.pose.pos = pos;
		z_rot.render(view, shader, true);

	} else if(action == Gui::Action::scale) {

		x_scale.pose.scale = scale;
		x_scale.pose.pos = pos + Vec3(0.15f * scl, 0.0f, 0.0f);
		x_scale.render(view, shader, true);

		y_scale.pose.scale = scale;
		y_scale.pose.pos = pos + Vec3(0.0f, 0.15f * scl, 0.0f);
		y_scale.render(view, shader, true);

		z_scale.pose.scale = scale;
		z_scale.pose.pos = pos + Vec3(0.0f, 0.0f, 0.15f * scl);
		z_scale.render(view, shader, true);
	}
}

void Scene::render_selected(Scene_Object& obj) {

	Vec3 prev_scale = obj.pose.scale;
	Vec3 prev_rot = obj.pose.euler;
	Vec3 prev_pos = obj.pose.pos;
	if(state.dragging) {
		if(state.action == Gui::Action::scale) obj.pose.scale = apply_action(obj);
		else if(state.action == Gui::Action::rotate) obj.pose.euler = apply_action(obj);
		else obj.pose.pos = apply_action(obj);
	}

	mesh_shader.bind();
	obj.render(view, mesh_shader);

	framebuffer.clear_d();
	obj.render(view, mesh_shader, false, true);
	
	Vec2 min, max;
	obj.bbox().screen_rect(viewproj, min, max);

	GL::flush_if_nvidia();
	GL::Effects::outline(framebuffer, framebuffer, Gui::outline, 
						 min - Vec2(3.0f / state.window_dim.x), 
						 max + Vec2(3.0f / state.window_dim.y));
	GL::flush_if_nvidia();
	
	framebuffer.clear_d();

	// TODO(max): this only scales correctly given a constant object position...
	float scl = (camera.pos() - obj.pose.pos).norm() / 5.0f;
	state.render_widgets(view, mesh_shader, obj.pose.pos, scl);

	obj.pose.scale = prev_scale;
	obj.pose.euler = prev_rot;
	obj.pose.pos = prev_pos;
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
			if(obj.first != state.drag_id) 
				obj.second.render(view, mesh_shader);
		}
	}
	{
		line_shader.bind();
		line_shader.uniform("viewproj", viewproj);
		baseplane.render();
	}

	auto selected = objs.find(state.drag_id);
	if(selected != objs.end()) {
		render_selected(selected->second);
	}

	framebuffer.blit_to(1, id_resolve, false);
	id_resolve.read(0, id_buffer);

	framebuffer.blit_to_screen(0, state.window_dim);
}

Vec3 Scene::apply_action(Scene_Object& obj) {

	Vec3 result;
	switch(state.action) {
	case Gui::Action::move: {
		result = obj.pose.pos + state.drag_end - state.drag_start;
	} break;
	case Gui::Action::rotate: {
		Vec3 axis;
		axis[(int)state.axis] = 1.0f;
		Quat rot = Quat::axis_angle(axis, state.drag_end[(int)state.axis]);
		Quat combined = rot * obj.pose.rotation_quat();
		result = combined.to_euler();
	} break;
	case Gui::Action::scale: {
		result = state.drag_end * obj.pose.scale;
	} break;
	default: assert(false);
	}

	return result;
}

Vec3 Scene::screen_to_world(Vec2 mouse) {

	Vec2 t(2.0f * mouse.x / state.window_dim.x - 1.0f, 
		   1.0f - 2.0f * mouse.y / state.window_dim.y);
	return (iviewproj * Vec3(t.x, t.y, 0.1f));
}

bool Scene::screen_to_axis(Scene_Object& obj, Vec2 mouse, Vec3& hit) {
	
	Vec3 axis; axis[(int)state.axis] = 1.0f;
	Vec3 dir = (screen_to_world(mouse) - camera.pos()).unit();
	
	Vec3 pos = state.dragging && state.action == Gui::Action::move ? apply_action(obj) : obj.pose.pos;
	Line select(camera.pos(), dir);
	Line target(pos, axis);
	Plane plane(pos, -camera.front().unit());

	bool ret = plane.hit(select, hit);
	hit = target.closest(hit);
	return ret && hit.valid();
 }

bool Scene::screen_to_plane(Scene_Object& obj, Vec2 mouse, Vec3& hit) {
	Vec3 world = screen_to_world(mouse);
	Vec3 cam = camera.pos();
	Line look(cam, (world - cam).unit());
	Vec3 axis; axis[(int)state.axis] = 1.0f;
	Vec3 pos = state.dragging && state.action == Gui::Action::move ? apply_action(obj) : obj.pose.pos;
	Plane p(pos, axis);
	return p.hit(look, hit);
}

void Scene::end_drag(Vec2 mouse) {

	if(!state.dragging) return;

	Scene_Object& obj = objs[state.drag_id];

	switch(state.action) {
	case Gui::Action::scale: {
		obj.pose.scale = apply_action(obj);
	} break;
	case Gui::Action::rotate: {
		obj.pose.euler = apply_action(obj); 
	} break;
	case Gui::Action::move: {
		obj.pose.pos = apply_action(obj);
	} break;
	default: assert(false);
	}

	state.drag_start = state.drag_end = {};
	state.dragging = false;
}

void Scene::drag(Vec2 mouse) {

	if(!state.dragging) return;
	
	Scene_Object& obj = objs[state.drag_id];
	Vec3 pos = obj.pose.pos;
	
	if(state.action == Gui::Action::rotate) {
	
		Vec3 hit;
		if(!screen_to_plane(obj, mouse, hit)) return;

		Vec3 ang = (hit - pos).unit();
		float sgn = sign(cross(state.drag_start, ang)[(int)state.axis]);
		state.drag_end = {};
		state.drag_end[(int)state.axis] = sgn * Degrees(std::acos(dot(state.drag_start, ang)));

		return;
	}

	Vec3 hit; 
	bool good;
	if(state.plane) good = screen_to_plane(obj, mouse, hit);
	else 			good = screen_to_axis(obj, mouse, hit);

	if(!good) return;

	if(state.action == Gui::Action::move) {
		state.drag_end = hit;
	} else if(state.action == Gui::Action::scale) {
		state.drag_end = {1.0f};
		state.drag_end[(int)state.axis] = (hit - pos).norm() / state.drag_start.norm();
	} else assert(false);
}

bool Scene::select(Vec2 mouse) {
	
	Scene_Object::ID clicked = read_id(mouse);
	state.dragging = true;
	state.plane = false;

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
	case (Scene_Object::ID)Gui::Basic::xy_trans: {
		state.action = Gui::Action::move;
		state.axis = Gui::Axis::Z;
		state.plane = true;
	} break;
	case (Scene_Object::ID)Gui::Basic::yz_trans: {
		state.action = Gui::Action::move;
		state.axis = Gui::Axis::X;
		state.plane = true;
	} break;
	case (Scene_Object::ID)Gui::Basic::xz_trans: {
		state.action = Gui::Action::move;
		state.axis = Gui::Axis::Y;
		state.plane = true;
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
		state.drag_id = clicked; 
	} break;
	}

	if(state.dragging) {
		Scene_Object& obj = objs[state.drag_id];
		
		Vec3 hit;
		if(state.action == Gui::Action::rotate) {
			if(screen_to_plane(obj, mouse, hit)) {
				state.drag_start = (hit - obj.pose.pos).unit();
				state.drag_end = {0.0f};
			}
			return state.dragging;
		}

		bool good;
		if(state.plane) good = screen_to_plane(obj, mouse, hit);
		else 			good = screen_to_axis(obj, mouse, hit);

		if(!good) return state.dragging;

		if(state.action == Gui::Action::move) {
			state.drag_start = state.drag_end = hit;
		} else {
			state.drag_start = hit; 
			state.drag_end = {1.0f};
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

	float available_w = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	float last_w = 0.0f, next_w = 0.0f;
	ImGuiStyle& style = ImGui::GetStyle();
	auto MaybeSameLineButton = [&](std::string label) -> bool {
		last_w = ImGui::GetItemRectMax().x;
		next_w = last_w +  style.ItemSpacing.x + ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2;
		if (next_w < available_w)
			ImGui::SameLine();
		return ImGui::Button(label.c_str());
	};

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

	if(MaybeSameLineButton("Load Object")) {
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
		
		bool selected = entry.first == state.drag_id;
		ImGui::SameLine();
		if(ImGui::Checkbox("##selected", &selected)) {
			if(selected) state.drag_id = entry.first;
			else 		 state.drag_id = 0;
		}

		auto fake_display = [&](Gui::Action mode, std::string label, Vec3& data, float sens) {
			if(state.dragging && state.action == mode) {
				Vec3 fake = apply_action(obj);
				ImGui::DragFloat3(label.c_str(), fake.data);
			} else {
				if(ImGui::DragFloat3(label.c_str(), data.data, sens) && selected)
					state.action = mode;
			}
		};
		
		obj.pose.clamp_euler();
		fake_display(Gui::Action::move, "Position", obj.pose.pos, 0.1f);
		fake_display(Gui::Action::rotate, "Rotation", obj.pose.euler, 1.0f);
		fake_display(Gui::Action::scale, "Scale", obj.pose.scale, 0.03f);
		
		if(selected) {
			if(ImGui::Button("Move"))
				state.action = Gui::Action::move;
			if(MaybeSameLineButton("Rotate"))
				state.action = Gui::Action::rotate;
			if(MaybeSameLineButton("Scale"))
				state.action = Gui::Action::scale;
			if(MaybeSameLineButton("Delete"))
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
		ImGui::Text("GPU: %s", GL::renderer().c_str());
		ImGui::Text("OpenGL: %s", GL::version().c_str());

		ImGui::End();
	}
}
