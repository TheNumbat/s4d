
#include "gui.h"
#include "scene/util.h"

#include <imgui/imgui.h>
#include <nfd/nfd.h>

Gui::Gui(Vec2 dim) : 
    baseplane(1.0f),
    widget_lines(1.0f),
	window_dim(dim) {

	x_trans = Scene_Object((Scene_Object::ID)Basic::x_trans, Pose::rotated({0.0f, 0.0f, -90.0f}), Util::arrow_mesh(), Gui::Color::red);
	y_trans = Scene_Object((Scene_Object::ID)Basic::y_trans, {}, Util::arrow_mesh(), Gui::Color::green);
	z_trans = Scene_Object((Scene_Object::ID)Basic::z_trans, Pose::rotated({90.0f, 0.0f, 0.0f}), Util::arrow_mesh(), Gui::Color::blue);

	xy_trans = Scene_Object((Scene_Object::ID)Basic::xy_trans, Pose::rotated({-90.0f, 0.0f, 0.0f}), Util::square_mesh(0.1f), Gui::Color::blue);
	yz_trans = Scene_Object((Scene_Object::ID)Basic::yz_trans, Pose::rotated({0.0f, 0.0f, -90.0f}), Util::square_mesh(0.1f), Gui::Color::red);
	xz_trans = Scene_Object((Scene_Object::ID)Basic::xz_trans, {}, Util::square_mesh(0.1f), Gui::Color::green);

	x_rot = Scene_Object((Scene_Object::ID)Basic::x_rot, Pose::rotated({0.0f, 0.0f, -90.0f}), Util::torus_mesh(0.975f, 1.0f), Gui::Color::red);
	y_rot = Scene_Object((Scene_Object::ID)Basic::y_rot, {}, Util::torus_mesh(0.975f, 1.0f), Gui::Color::green);
	z_rot = Scene_Object((Scene_Object::ID)Basic::z_rot, Pose::rotated({90.0f, 0.0f, 0.0f}), Util::torus_mesh(0.975f, 1.0f), Gui::Color::blue);

	x_scale = Scene_Object((Scene_Object::ID)Basic::x_scale, Pose::rotated({0.0f, 0.0f, -90.0f}), Util::scale_mesh(), Gui::Color::red);
	y_scale = Scene_Object((Scene_Object::ID)Basic::y_scale, {}, Util::scale_mesh(), Gui::Color::green);
	z_scale = Scene_Object((Scene_Object::ID)Basic::z_scale, Pose::rotated({90.0f, 0.0f, 0.0f}), Util::scale_mesh(), Gui::Color::blue);

    create_baseplane();
}

Gui::~Gui() {

}

Scene_Object::ID Gui::num_ids() {
    return (Scene_Object::ID)Basic::count;
}

void Gui::update_dim(Vec2 dim) {
    window_dim = dim;
}

Vec3 Gui::Color::axis(Axis a) {
	switch(a) {
	case Axis::X: return red;
	case Axis::Y: return green;
	case Axis::Z: return blue;
	default: assert(false);
	}
	return Vec3();
}

bool Gui::keydown(SDL_Keycode key) {
    if(key == SDLK_e && selected) {
        action = (Gui::Action)(((int)action + 1) % 3);
		return true;
    }
	return false;
}

void Gui::generate_widget_lines(const Scene_Object& obj) {
	auto add_axis = [&](int axis) {
		Vec3 start = obj.pose.pos; start[axis] -= 10000.0f;
		Vec3 end = obj.pose.pos; end[axis] += 10000.0f;
		Vec3 color = Color::axis((Axis)axis);
		widget_lines.add(start, end, color);
	};
	if(drag_plane) {
		add_axis(((int)axis + 1) % 3);
		add_axis(((int)axis + 2) % 3);
	} else {
		add_axis((int)axis);
	}
}

Scene_Object::ID Gui::selected_id() {
	return selected;
}

void Gui::render_widgets(Mat4 view, const GL::Shader& line, const GL::Shader& mesh, const Pose& pose, float scl) {

	Vec3 scale(scl);

	line.bind();
	line.uniform("alpha", 0.5f);
	widget_lines.render();

	mesh.bind();
	if(action == Action::move) {

		if(dragging) return;

		x_trans.pose.scale = scale;
		x_trans.pose.pos = pose.pos + Vec3(0.15f * scl, 0.0f, 0.0f);
		x_trans.render(view, mesh, true);

		y_trans.pose.scale = scale;
		y_trans.pose.pos = pose.pos + Vec3(0.0f, 0.15f * scl, 0.0f);
		y_trans.render(view, mesh, true);

		z_trans.pose.scale = scale;
		z_trans.pose.pos = pose.pos + Vec3(0.0f, 0.0f, 0.15f * scl);
		z_trans.render(view, mesh, true);

		GL::disable(GL::Opt::culling);
		xy_trans.pose.scale = scale;
		xy_trans.pose.pos = pose.pos + Vec3(0.45f * scl, 0.45f * scl, 0.0f);
		xy_trans.render(view, mesh, true);

		yz_trans.pose.scale = scale;
		yz_trans.pose.pos = pose.pos + Vec3(0.0f, 0.45f * scl, 0.45f * scl);
		yz_trans.render(view, mesh, true);

		xz_trans.pose.scale = scale;
		xz_trans.pose.pos = pose.pos + Vec3(0.45f * scl, 0.0f, 0.45f * scl);
		xz_trans.render(view, mesh, true);
		GL::enable(GL::Opt::culling);
	
	} else if(action == Action::rotate) {

		if(!dragging || axis == Axis::X) {
			x_rot.pose.scale = scale;
			x_rot.pose.pos = pose.pos;
			x_rot.render(view, mesh, true);
		}
		if(!dragging || axis == Axis::Y) {
			y_rot.pose.scale = scale;
			y_rot.pose.pos = pose.pos;
			y_rot.render(view, mesh, true);
		}
		if(!dragging || axis == Axis::Z) {
			z_rot.pose.scale = scale;
			z_rot.pose.pos = pose.pos;
			z_rot.render(view, mesh, true);
		}

	} else if(action == Action::scale) {

		if(dragging) return;

		x_scale.pose.scale = scale;
		x_scale.pose.pos = pose.pos + Vec3(0.15f * scl, 0.0f, 0.0f);
		x_scale.render(view, mesh, true);

		y_scale.pose.scale = scale;
		y_scale.pose.pos = pose.pos + Vec3(0.0f, 0.15f * scl, 0.0f);
		y_scale.render(view, mesh, true);

		z_scale.pose.scale = scale;
		z_scale.pose.pos = pose.pos + Vec3(0.0f, 0.0f, 0.15f * scl);
		z_scale.render(view, mesh, true);
	
	} else assert(false);
}

void Gui::objs(Undo& undo, Scene& scene, float menu_height) {

	const ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::SetNextWindowPos({0.0, menu_height});
	ImGui::SetNextWindowSize({window_dim.x / 5.0f, window_dim.y});

	ImGui::Begin("Objects", nullptr, flags);

	float available_w = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	float last_w = 0.0f, next_w = 0.0f;
	
	auto wrap_button = [&](std::string label) -> bool {
		last_w = ImGui::GetItemRectMax().x;
		next_w = last_w + style.ItemSpacing.x + ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2;
		if (next_w < available_w)
			ImGui::SameLine();
		return ImGui::Button(label.c_str());
	};
	auto state_button = [&](Action act, std::string name, bool same = true) -> bool {
		bool active = act == action;
		if(active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
		bool clicked = same ? wrap_button(name) : ImGui::Button(name.c_str());
		if(active) ImGui::PopStyleColor();
		return clicked;
	};

	if(ImGui::Button("Create Object")) {
		ImGui::OpenPopup("Type");
	}
	if(ImGui::BeginPopup("Type")) {
		
		if(ImGui::Button("Cube")) {
			undo.add_obj(scene, Util::cube_mesh(1.0f));
			ImGui::CloseCurrentPopup();
		}

		if(ImGui::Button("Cylinder")) {
			undo.add_obj(scene, Util::cyl_mesh(1.0f, 1.0f));
			ImGui::CloseCurrentPopup();
		}

		if(ImGui::Button("Torus")) {
			undo.add_obj(scene, Util::torus_mesh(0.8f, 1.0f));
			ImGui::CloseCurrentPopup();
		}

		if(ImGui::Button("Sphere")) {
			undo.add_obj(scene, Util::sphere_mesh(1.0f));
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if(wrap_button("Load Object")) {
		char* path = nullptr;
		NFD_OpenDialog("obj", nullptr, &path);
		
		if(path) {
			GL::Mesh new_mesh;
			std::string error = Util::obj_mesh(std::string(path), new_mesh);
			if(error.empty()) {
				undo.add_obj(scene, std::move(new_mesh));
			} else {
				set_error(error);
			}
			free(path);
		}
	}

	if(!scene.empty())
		ImGui::Separator();

	size_t i = 0;
	Scene_Object::ID to_delete = 0;

	scene.for_objs([&](Scene_Object& obj) {

		ImGui::PushID(obj.id());

		std::string& name = obj.opt.name;
		ImGui::InputText("##name", name.data(), name.capacity());
		
		bool is_selected = obj.id() == selected;
		ImGui::SameLine();
		if(ImGui::Checkbox("##selected", &is_selected)) {
			if(is_selected) selected = obj.id();
			else 		    selected = 0;
		}

		auto check_undo = [&](Action mode, Vec3& old, const Vec3& newv) {
			if (ImGui::IsItemActivated()) {
				old = newv;
			}
			if (ImGui::IsItemDeactivatedAfterEdit() && old != newv) {
				Pose p = obj.pose;
				if(mode == Action::move) {p.pos = newv; obj.pose.pos = old;}
				else if(mode == Action::rotate) {p.euler = newv; obj.pose.euler = old;}
				else if(mode == Action::scale) {p.scale = newv; obj.pose.scale = old;}
				undo.update_obj(scene, obj.id(), p);
			}
		};

		auto fake_display = [&](Action mode, std::string label, Vec3& data, float sens) {
			if(dragging && action == mode) {
				Vec3 fake = apply_action(obj);
				ImGui::DragFloat3(label.c_str(), fake.data);
			} else if(ImGui::DragFloat3(label.c_str(), data.data, sens)) {
				if(is_selected) {
					action = mode;
				}
			}
			check_undo(mode, gui_drag_start, data);
		};
		
		obj.pose.clamp_euler();
		fake_display(Action::move, "Position", obj.pose.pos, 0.1f);
		fake_display(Action::rotate, "Rotation", obj.pose.euler, 1.0f);
		fake_display(Action::scale, "Scale", obj.pose.scale, 0.03f);
		
		if(is_selected) {
			if(state_button(Action::move, "Move", false))
				action = Action::move;
			if(state_button(Action::rotate, "Rotate"))
				action = Action::rotate;
			if(state_button(Action::scale, "Scale"))
				action = Action::scale;
			if(wrap_button("Delete"))
				to_delete = obj.id();
		}

		if(i++ != scene.size() - 1) ImGui::Separator();

		ImGui::PopID();
	});

	if(to_delete) undo.del_obj(scene, to_delete);

	ImGui::End();
}

void Gui::set_error(std::string msg) {
	error_msg = msg;
	error_shown = true;
}

void Gui::error() {
	if(error_shown) {
		Vec2 center = window_dim / 2.0f;
		ImGui::SetNextWindowPos(ImVec2{center.x, center.y}, 0, ImVec2{0.5f, 0.5f});
		ImGui::Begin("Error", &error_shown, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
		ImGui::Text("%s", error_msg.c_str());
		if(ImGui::Button("Close")) {
			error_shown = false;
		}
		ImGui::End();
	}
}

float Gui::menu(Undo& undo, bool& settings) {

	auto state_button = [&](Gui::Mode m, std::string name) -> bool {
		bool active = m == mode;
		if(active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
		bool clicked = ImGui::Button(name.c_str());
		if(active) ImGui::PopStyleColor();
		return clicked;
	};

	float menu_height = 0.0f;
	if(ImGui::BeginMainMenuBar()) {

		if(ImGui::BeginMenu("File")) {

			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Edit")) {

			if(ImGui::MenuItem("Undo (Ctrl+z)")) {
				undo.undo();
			}
			if(ImGui::MenuItem("Redo (Ctrl+y)")) {
				undo.redo();
			}
			if(ImGui::MenuItem("Display Settings")) {
				settings = true;
			}
			ImGui::EndMenu();
		}

		if(state_button(Gui::Mode::scene, "Scene"))
			mode = Gui::Mode::scene;

		if(state_button(Gui::Mode::model, "Model"))
			mode = Gui::Mode::model;

		if(state_button(Gui::Mode::render, "Render"))
			mode = Gui::Mode::render;

		if(state_button(Gui::Mode::rig, "Rig"))
			mode = Gui::Mode::rig;

		if(state_button(Gui::Mode::animate, "Animate"))
			mode = Gui::Mode::animate;

		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);

		menu_height = ImGui::GetWindowSize().y;
		ImGui::EndMainMenuBar();
	}

	return menu_height;
}

void Gui::create_baseplane() {

	const int R = 25;
	for(int i = -R; i <= R; i++) {
		if(i == 0) {
			baseplane.add({-R, 0, i}, {R, 0, i}, Color::red);
			baseplane.add({i, 0, -R}, {i, 0, R}, Color::blue);
			continue;
		}
		baseplane.add({i, 0, -R}, {i, 0, R}, Color::baseplane);
		baseplane.add({-R, 0, i}, {R, 0, i}, Color::baseplane);
	}
}

Vec3 Gui::apply_action(const Scene_Object& obj) {

	Vec3 result, vaxis;
	vaxis[(int)axis] = 1.0f;

	switch(action) {
	case Action::move: {
		result = obj.pose.pos + drag_end - drag_start;
	} break;
	case Action::rotate: {
		Quat rot = Quat::axis_angle(vaxis, drag_end[(int)axis]);
		Quat combined = rot * obj.pose.rotation_quat();
		result = combined.to_euler();
	} break;
	case Action::scale: {
		result = {1.0f};
		result[(int)axis] = drag_end[(int)axis];
		Mat4 rot = obj.pose.rotation_mat();
		Mat4 trans = Mat4::transpose(rot) * Mat4::scale(result) * rot * Mat4::scale(obj.pose.scale);
		result = Vec3(trans[0][0], trans[1][1], trans[2][2]);
	} break;
	default: assert(false);
	}

	return result;
}

bool Gui::to_axis(const Scene_Object& obj, Vec3 pos, Vec3 dir, Vec3& hit) {
	
	Vec3 axis1; axis1[(int)axis] = 1.0f;
	Vec3 axis2; axis2[((int)axis + 1) % 3] = 1.0f;
	Vec3 axis3; axis3[((int)axis + 2) % 3] = 1.0f;
	
	Vec3 opos = obj.pose.pos;
	Line select(pos, dir);
	Line target(opos, axis1);
	Plane l(opos, axis2);
	Plane r(opos, axis3);

	Vec3 hit1, hit2;
	bool hl = l.hit(select, hit1);
	bool hr = r.hit(select, hit2);
	if(!hl && !hr) return false;
	else if(!hl) hit = hit2;
	else if(!hr) hit = hit1;
	else hit = (hit1 - pos).norm() > (hit2 - pos).norm() ? hit2 : hit1;
	
	hit = target.closest(hit);
	return hit.valid();
 }

bool Gui::to_plane(const Scene_Object& obj, Vec3 pos, Vec3 dir, Vec3& hit) {

	Line look(pos, dir);
	Vec3 vaxis; vaxis[(int)axis] = 1.0f;
	Plane p(obj.pose.pos, vaxis);
	return p.hit(look, hit);
}

void Gui::end_drag(Undo& undo, Scene& scene) {

	if(!dragging) return;
    
    Scene_Object& obj = *scene.get(selected);
	Pose p = obj.pose;

	switch(action) {
	case Action::scale: {
		p.scale = apply_action(obj);
	} break;
	case Action::rotate: {
		p.euler = apply_action(obj); 
	} break;
	case Action::move: {
		p.pos = apply_action(obj);
	} break;
	default: assert(false);
	}

	widget_lines.clear();
	drag_start = drag_end = {};
	dragging = false;

	undo.update_obj(scene, obj.id(), p);
}

void Gui::drag_to(Scene& scene, Vec3 cam, Vec3 dir) {

	if(!dragging) return;
	
    Scene_Object& obj = *scene.get(selected);
	Vec3 pos = obj.pose.pos;
	
	if(action == Action::rotate) {
	
		Vec3 hit;
		if(!to_plane(obj, cam, dir, hit)) return;

		Vec3 ang = (hit - pos).unit();
		float sgn = sign(cross(drag_start, ang)[(int)axis]);
		drag_end = {};
		drag_end[(int)axis] = sgn * Degrees(std::acos(dot(drag_start, ang)));

		return;
	}

	Vec3 hit; 
	bool good;
	if(drag_plane) good = to_plane(obj, cam, dir, hit);
	else 	       good = to_axis(obj, cam, dir, hit);

	if(!good) return;

	if(action == Action::move) {
		drag_end = hit;
	} else if(action == Action::scale) {
		drag_end = {1.0f};
		drag_end[(int)axis] = (hit - pos).norm() / (drag_start - pos).norm();
	} else assert(false);
}

bool Gui::select(Scene& scene, Scene_Object::ID id, Vec3 cam, Vec3 dir) {
	
	dragging = true;
	drag_plane = false;

	switch(id) {
	case (Scene_Object::ID)Basic::x_trans: {
		action = Action::move;
		axis = Axis::X;
	} break;
	case (Scene_Object::ID)Basic::y_trans: {
		action = Action::move;
		axis = Axis::Y;
	} break;
	case (Scene_Object::ID)Basic::z_trans: {
		action = Action::move;
		axis = Axis::Z;
	} break;
	case (Scene_Object::ID)Basic::xy_trans: {
		action = Action::move;
		axis = Axis::Z;
		drag_plane = true;
	} break;
	case (Scene_Object::ID)Basic::yz_trans: {
		action = Action::move;
		axis = Axis::X;
		drag_plane = true;
	} break;
	case (Scene_Object::ID)Basic::xz_trans: {
		action = Action::move;
		axis = Axis::Y;
		drag_plane = true;
	} break;
	case (Scene_Object::ID)Basic::x_rot: {
		action = Action::rotate;
		axis = Axis::X;
	} break;
	case (Scene_Object::ID)Basic::y_rot: {
		action = Action::rotate;
		axis = Axis::Y;
	} break;
	case (Scene_Object::ID)Basic::z_rot: {
		action = Action::rotate;
		axis = Axis::Z;
	} break;
	case (Scene_Object::ID)Basic::x_scale: {
		action = Action::scale;
		axis = Axis::X;
	} break;
	case (Scene_Object::ID)Basic::y_scale: {
		action = Action::scale;
		axis = Axis::Y;
	} break;
	case (Scene_Object::ID)Basic::z_scale: {
		action = Action::scale;
		axis = Axis::Z;
	} break;
	default: {
		dragging = false;
		selected = id; 
	} break;
	}

	if(dragging) {
		Scene_Object& obj = *scene.get(selected);
		Vec3 hit;
		if(action == Action::rotate) {
			if(to_plane(obj, cam, dir, hit)) {
				drag_start = (hit - obj.pose.pos).unit();
				drag_end = {0.0f};
			}
			return selected;
		}

		bool good;
		if(drag_plane) good = to_plane(obj, cam, dir, hit);
		else           good = to_axis(obj, cam, dir, hit);

		if(!good) return selected;

		if(action == Action::move) {
			
			drag_start = drag_end = hit;

		} else {
			drag_start = hit;
			drag_end = {1.0f};
		}
		generate_widget_lines(obj);
	}
	return selected;
}

void Gui::apply_transform(Scene_Object& obj) {
    if(dragging) {
			 if(action == Gui::Action::scale)  obj.pose.scale = apply_action(obj);
		else if(action == Gui::Action::rotate) obj.pose.euler = apply_action(obj);
		else if(action == Gui::Action::move)   obj.pose.pos   = apply_action(obj);
		else assert(false);
	}
}

void Gui::render_base() {
	baseplane.render();
}
