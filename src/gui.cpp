
#include "gui.h"
#include "scene/util.h"
#include "scene/mesh_render.h"

#include <imgui/imgui.h>
#include <nfd/nfd.h>

Gui::Gui(Vec2 dim) : 
	baseplane(1.0f),
	widget_lines(1.0f),
	window_dim(dim) {

	x_trans = Scene_Object((Scene_Object::ID)Basic::x_trans, Pose::rotated({0.0f, 0.0f, -90.0f}), Util::arrow_mesh(0.03f, 0.075f, 1.0f), Gui::Color::red);
	y_trans = Scene_Object((Scene_Object::ID)Basic::y_trans, {}, Util::arrow_mesh(0.03f, 0.075f, 1.0f), Gui::Color::green);
	z_trans = Scene_Object((Scene_Object::ID)Basic::z_trans, Pose::rotated({90.0f, 0.0f, 0.0f}), Util::arrow_mesh(0.03f, 0.075f, 1.0f), Gui::Color::blue);

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

bool Gui::keydown(Undo& undo, Scene& scene, SDL_Keycode key) {
	if(key == SDLK_r && selected_mesh) {
		action = (Gui::Action)(((int)action + 1) % 4);	
		return true;
	}
	if(key == SDLK_DELETE && selected_mesh) {
		undo.del_obj(scene, selected_mesh);
		selected_mesh = 0;
	}
	if(_mode == Mode::model) {
		auto sel = Renderer::he_selected();
		if(sel.has_value()) {
			if(key == SDLK_h) {
				std::visit(overloaded {
					[&](Halfedge_Mesh::VertexRef vert) {
						Renderer::set_he_select(vert->halfedge());
					},
					[&](Halfedge_Mesh::EdgeRef edge) {
						Renderer::set_he_select(edge->halfedge());
					},
					[&](Halfedge_Mesh::FaceRef face) {
						Renderer::set_he_select(face->halfedge());
					},
					[&](auto) {}
				}, *sel);
			} else if(std::holds_alternative<Halfedge_Mesh::HalfedgeRef>(*sel)) {
				auto h = std::get<Halfedge_Mesh::HalfedgeRef>(*sel);
				if(key == SDLK_n) {
					Renderer::set_he_select(h->next());
				} else if(key == SDLK_t) {
					Renderer::set_he_select(h->twin());
				} else if(key == SDLK_v) {
					Renderer::set_he_select(h->vertex());
				} else if(key == SDLK_e) {
					Renderer::set_he_select(h->edge());
				} else if(key == SDLK_f) {
					Renderer::set_he_select(h->face());
				}
			}
		}
	}
	return false;
}

void Gui::generate_widget_lines(Vec3 pos) {
	auto add_axis = [&](int axis) {
		Vec3 start = pos; start[axis] -= 10000.0f;
		Vec3 end = pos; end[axis] += 10000.0f;
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
	return selected_mesh;
}

void Gui::render_widgets(Mat4 viewproj, Mat4 view, Vec3 pos, float scl) {

	Renderer::reset_depth();

	Vec3 scale(scl);
	Renderer::lines(widget_lines, viewproj, 0.5f);

	if(action == Action::move) {

		if(dragging) return;

		x_trans.pose.scale = scale;
		x_trans.pose.pos = pos + Vec3(0.15f * scl, 0.0f, 0.0f);
		x_trans.render_mesh(view, true);

		y_trans.pose.scale = scale;
		y_trans.pose.pos = pos + Vec3(0.0f, 0.15f * scl, 0.0f);
		y_trans.render_mesh(view, true);

		z_trans.pose.scale = scale;
		z_trans.pose.pos = pos + Vec3(0.0f, 0.0f, 0.15f * scl);
		z_trans.render_mesh(view, true);

		xy_trans.pose.scale = scale;
		xy_trans.pose.pos = pos + Vec3(0.45f * scl, 0.45f * scl, 0.0f);
		xy_trans.render_mesh(view, true);

		yz_trans.pose.scale = scale;
		yz_trans.pose.pos = pos + Vec3(0.0f, 0.45f * scl, 0.45f * scl);
		yz_trans.render_mesh(view, true);

		xz_trans.pose.scale = scale;
		xz_trans.pose.pos = pos + Vec3(0.45f * scl, 0.0f, 0.45f * scl);
		xz_trans.render_mesh(view, true);
	
	} else if(action == Action::rotate) {

		if(!dragging || axis == Axis::X) {
			x_rot.pose.scale = scale;
			x_rot.pose.pos = pos;
			x_rot.render_mesh(view, true);
		}
		if(!dragging || axis == Axis::Y) {
			y_rot.pose.scale = scale;
			y_rot.pose.pos = pos;
			y_rot.render_mesh(view, true);
		}
		if(!dragging || axis == Axis::Z) {
			z_rot.pose.scale = scale;
			z_rot.pose.pos = pos;
			z_rot.render_mesh(view, true);
		}

	} else if(action == Action::scale) {

		if(dragging) return;

		x_scale.pose.scale = scale;
		x_scale.pose.pos = pos + Vec3(0.15f * scl, 0.0f, 0.0f);
		x_scale.render_mesh(view, true);

		y_scale.pose.scale = scale;
		y_scale.pose.pos = pos + Vec3(0.0f, 0.15f * scl, 0.0f);
		y_scale.render_mesh(view, true);

		z_scale.pose.scale = scale;
		z_scale.pose.pos = pos + Vec3(0.0f, 0.0f, 0.15f * scl);
		z_scale.render_mesh(view, true);
	
	} else if(action == Action::bevel) {

	} else assert(false);
}

void Gui::write_scene(Scene& scene) {

	char* path = nullptr;
	NFD_SaveDialog("dae", nullptr, &path);
	if(path) {
		std::string error = scene.write(std::string(path));
		if(!error.empty()) {
			set_error(error);
		}
		free(path);
	}
}

void Gui::load_scene(Scene& scene, Undo& undo) {

	char* path = nullptr;
	NFD_OpenDialog(file_types, nullptr, &path);
	
	if(path) {
		std::string error = scene.load(true, undo, std::string(path));
		if(!error.empty()) {
			set_error(error);
		}
		free(path);
	}
}

	
bool Gui::wrap_button(std::string label) {
	ImGuiStyle& style = ImGui::GetStyle();
	float available_w = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	float last_w = ImGui::GetItemRectMax().x;
	float next_w = last_w + style.ItemSpacing.x + ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2;
	if (next_w < available_w)
		ImGui::SameLine();
	return ImGui::Button(label.c_str());
};

bool Gui::action_button(Action act, std::string name, bool same) {
	bool active = act == action;
	if(active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
	bool clicked = same ? wrap_button(name) : ImGui::Button(name.c_str());
	if(active) ImGui::PopStyleColor();
	return clicked;
};

void Gui::objs(Scene& scene, Undo& undo, float menu_height) {

	const ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

	ImGui::SetNextWindowPos({0.0, menu_height});
	ImGui::SetNextWindowSize({window_dim.x / 5.0f, window_dim.y});

	ImGui::Begin("Objects", nullptr, flags);

	if(_mode == Mode::scene) {
		ImGui::Text("Edit Scene");

		if(ImGui::Button("Load Scene")) {
			load_scene(scene, undo);
		}
		if(wrap_button("Export Scene")) {
			write_scene(scene);
		}

		if(ImGui::Button("Add Objects")) {
			char* path = nullptr;
			NFD_OpenDialog(file_types, nullptr, &path);

			if(path) {
				std::string error = scene.load(false, undo, std::string(path));
				if(!error.empty()) {
					set_error(error);
				}
				free(path);
			}
		}
	}

	if(!scene.empty()) {
		ImGui::Separator();	
		ImGui::Text("Select an Object");
	}

	scene.for_objs([&](Scene_Object& obj) {

		ImGui::PushID(obj.id());

		std::string& name = obj.opt.name;
		ImGui::InputText("##name", name.data(), name.capacity());
		
		bool is_selected = obj.id() == selected_mesh;
		ImGui::SameLine();
		if(ImGui::Checkbox("##selected", &is_selected)) {
			if(is_selected) selected_mesh = obj.id();
			else 		    selected_mesh = 0;
		}

		ImGui::PopID();
	});

	if(_mode == Mode::scene && selected_mesh) {

		Scene_Object& obj = *scene.get(selected_mesh);

		auto check_undo = [&](Action act, const Vec3& newv) {
			static Vec3 old;
			if (ImGui::IsItemActivated()) {
				old = newv;
			}
			if (ImGui::IsItemDeactivatedAfterEdit() && old != newv) {
				Pose p = obj.pose;
				if(act == Action::move) {p.pos = newv; obj.pose.pos = old;}
				else if(act == Action::rotate) {p.euler = newv; obj.pose.euler = old;}
				else if(act == Action::scale) {p.scale = newv; obj.pose.scale = old;}
				undo.update_obj(scene, obj.id(), p);
			}
		};

		auto fake_display = [&](Action act, std::string label, Vec3& data, float sens) {
			if(dragging && action == act) {
				Pose fake = apply_action(obj.pose);
				if(action == Action::rotate) fake.euler = fake.euler.range(0.0f, 360.0f);
				ImGui::DragFloat3(label.c_str(), fake.euler.data);
			} else if(ImGui::DragFloat3(label.c_str(), data.data, sens)) {
				action = act;
			}
			check_undo(act, data);
		};

		ImGui::Separator();
		ImGui::Text("Edit Object");

		obj.pose.clamp_euler();
		fake_display(Action::move, "Position", obj.pose.pos, 0.1f);
		fake_display(Action::rotate, "Rotation", obj.pose.euler, 1.0f);
		fake_display(Action::scale, "Scale", obj.pose.scale, 0.03f);

		if(ImGui::Button("Edit Mesh"))
			_mode = Mode::model;
		if(wrap_button("Delete")) {
			undo.del_obj(scene, selected_mesh);
			selected_mesh = 0;
		}	

		if(action_button(Action::move, "Move", false))
			action = Action::move;
		if(action_button(Action::rotate, "Rotate"))
			action = Action::rotate;
		if(action_button(Action::scale, "Scale"))
			action = Action::scale;

	} else if(_mode == Mode::model) {

		auto sel = Renderer::he_selected();
		
		if(selected_mesh) {

			Scene_Object& obj = *scene.get(selected_mesh);
			Halfedge_Mesh& mesh = obj.get_mesh();
			Halfedge_Mesh before;
			mesh.copy_to(before);
			unsigned int before_id = Renderer::get_he_select();

			bool update_mesh = false;
			bool update_ref = false;
			Halfedge_Mesh::ElementRef new_ref;

			ImGui::Separator();
			ImGui::Text("Global Operations");
			if(ImGui::Button("Triangulate")) {
				mesh.triangulate();
				update_mesh = true;
			}
			ImGui::Separator();

			if(sel.has_value()) {
				
				ImGui::Text("Local Operations");
				if(action_button(Action::move, "Move", false))
					action = Action::move;
				if(action_button(Action::rotate, "Rotate"))
					action = Action::rotate;
				if(action_button(Action::scale, "Scale"))
					action = Action::scale;
				if(action_button(Action::bevel, "Bevel"))
					action = Action::bevel;

				std::visit(overloaded {
					[&](Halfedge_Mesh::VertexRef vert) {
						if(ImGui::Button("Erase")) {
							new_ref = mesh.erase_vertex(vert);
							update_mesh = true;
							update_ref = true;
						}
					},
					[&](Halfedge_Mesh::EdgeRef edge) {
						if(ImGui::Button("Erase")) {
							new_ref = mesh.erase_edge(edge);
							update_mesh = true;
							update_ref = true;
						}
						if(wrap_button("Collapse")) {
							new_ref = mesh.collapse_edge(edge);
							update_mesh = true;
							update_ref = true;
						}
						if(wrap_button("Flip")) {
							new_ref = mesh.flip_edge(edge);
							update_mesh = true;
							update_ref = true;
						}
						if(wrap_button("Split")) {
							new_ref = mesh.split_edge(edge);
							update_mesh = true;
							update_ref = true;
						}
					},
					[&](Halfedge_Mesh::FaceRef face) {
						if(ImGui::Button("Collapse")) {
							new_ref = mesh.collapse_face(face);
							update_mesh = true;
							update_ref = true;
						}
					},
					[&](auto) {}
				}, *sel);

				ImGui::Separator();
				ImGui::Text("Navigation");
				std::visit(overloaded {
					[&](Halfedge_Mesh::VertexRef vert) {
						if(ImGui::Button("Halfedge")) {
							Renderer::set_he_select(vert->halfedge());
						}
					},
					[&](Halfedge_Mesh::EdgeRef edge) {
						if(ImGui::Button("Halfedge")) {
							Renderer::set_he_select(edge->halfedge());
						}
					},
					[&](Halfedge_Mesh::FaceRef face)  {
						if(ImGui::Button("Halfedge")) {
							Renderer::set_he_select(face->halfedge());
						}
					},
					[&](Halfedge_Mesh::HalfedgeRef halfedge) {
						if(ImGui::Button("Vertex")) {
							Renderer::set_he_select(halfedge->vertex());
						}
						if(wrap_button("Edge")) {
							Renderer::set_he_select(halfedge->edge());
						}
						if(wrap_button("Face")) {
							Renderer::set_he_select(halfedge->face());
						}
						if(ImGui::Button("Twin")) {
							Renderer::set_he_select(halfedge->twin());
						}
						if(wrap_button("Next")) {
							Renderer::set_he_select(halfedge->next());
						}
					}
				}, *sel);
			}
			ImGui::Separator();

			if(update_mesh) {
				std::string err = mesh.validate();
				if(!err.empty()) {
					set_error(err);
					obj.set_mesh(before);
				} else {
					Renderer::dirty();
					Renderer::set_he_select(new_ref);
					obj.set_mesh_dirty();
					undo.update_mesh(scene, selected_mesh, std::move(before), before_id);
				}
			}
		}
	}

	ImGui::End();
}

void Gui::set_error(std::string msg) {
	error_msg = msg;
	error_shown = true;
}

void Gui::error() {
	if(error_shown) {
		Vec2 center = window_dim / 2.0f;
		ImGui::SetNextWindowPos(Vec2{center.x, center.y}, 0, Vec2{0.5f, 0.5f});
		ImGui::Begin("Errors", &error_shown, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
		ImGui::Text("%s", error_msg.c_str());
		if(ImGui::Button("Close")) {
			error_shown = false;
		}
		ImGui::End();
	}
}

bool Gui::mode_button(Gui::Mode m, std::string name) {
	bool active = m == _mode;
	if(active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
	bool clicked = ImGui::Button(name.c_str());
	if(active) ImGui::PopStyleColor();
	return clicked;
};

float Gui::menu(Scene& scene, Undo& undo, bool& settings) {

	float menu_height = 0.0f;
	if(ImGui::BeginMainMenuBar()) {

		if(ImGui::BeginMenu("File")) {

			if(ImGui::MenuItem("Load Scene")) {
				load_scene(scene, undo);
			}
			if(ImGui::MenuItem("Export Scene")) {
				write_scene(scene);
			}
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

		if(mode_button(Gui::Mode::scene, "Scene"))
			_mode = Gui::Mode::scene;

		if(mode_button(Gui::Mode::model, "Model"))
			_mode = Gui::Mode::model;

		// if(mode_button(Gui::Mode::render, "Render"))
		// 	_mode = Gui::Mode::render;

		// if(mode_button(Gui::Mode::rig, "Rig"))
		// 	_mode = Gui::Mode::rig;

		// if(mode_button(Gui::Mode::animate, "Animate"))
		// 	_mode = Gui::Mode::animate;

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

Pose Gui::apply_action(const Pose& pose) {

	Pose result = pose;
	Vec3 vaxis;
	vaxis[(int)axis] = 1.0f;

	switch(action) {
	case Action::move: {
		result.pos = pose.pos + drag_end - drag_start;
	} break;
	case Action::rotate: {
		Quat rot = Quat::axis_angle(vaxis, drag_end[(int)axis]);
		Quat combined = rot * pose.rotation_quat();
		result.euler = combined.to_euler();
	} break;
	case Action::scale: {
		result.scale = {1.0f};
		result.scale[(int)axis] = drag_end[(int)axis];
		Mat4 rot = pose.rotation_mat();
		Mat4 trans = Mat4::transpose(rot) * Mat4::scale(result.scale) * rot * Mat4::scale(pose.scale);
		result.scale = Vec3(trans[0][0], trans[1][1], trans[2][2]);
	} break;
	case Action::bevel: {
		Vec2 off = bevel_start - bevel_end;
		result.pos = 2.0f * Vec3(off.x, -off.y, 0.0f);
	} break;
	default: assert(false);
	}

	return result;
}

bool Gui::to_axis(Vec3 obj_pos, Vec3 cam_pos, Vec3 dir, Vec3& hit) {
	
	Vec3 axis1; axis1[(int)axis] = 1.0f;
	Vec3 axis2; axis2[((int)axis + 1) % 3] = 1.0f;
	Vec3 axis3; axis3[((int)axis + 2) % 3] = 1.0f;
	
	Line select(cam_pos, dir);
	Line target(obj_pos, axis1);
	Plane l(obj_pos, axis2);
	Plane r(obj_pos, axis3);

	Vec3 hit1, hit2;
	bool hl = l.hit(select, hit1);
	bool hr = r.hit(select, hit2);
	if(!hl && !hr) return false;
	else if(!hl) hit = hit2;
	else if(!hr) hit = hit1;
	else hit = (hit1 - cam_pos).norm() > (hit2 - cam_pos).norm() ? hit2 : hit1;
	
	hit = target.closest(hit);
	return hit.valid();
 }

bool Gui::to_plane(Vec3 obj_pos, Vec3 cam_pos, Vec3 dir, Vec3 norm, Vec3& hit) {

	Line look(cam_pos, dir);
	Plane p(obj_pos, norm);
	return p.hit(look, hit);
}

bool Gui::start_drag(Vec3 pos, Vec3 cam, Vec2 spos, Vec3 dir) {
	
	Vec3 hit;
	Vec3 norm; norm[(int)axis] = 1.0f;

	if(action == Action::rotate) {
		if(to_plane(pos, cam, dir, norm, hit)) {
			drag_start = (hit - pos).unit();
			drag_end = {0.0f};
		}
	} else {

		bool good;
		
		auto elem = Renderer::he_selected();

		if(drag_plane) good = to_plane(pos, cam, dir, norm, hit);
		else 	       good = to_axis(pos, cam, dir, hit);

		if(!good) return dragging;

		if(action == Action::bevel) {
			bevel_start = bevel_end = spos;
		} if(action == Action::move) {
			drag_start = drag_end = hit;
		} else {
			drag_start = hit;
			drag_end = {1.0f};
		}

		if(action != Action::bevel)
			generate_widget_lines(pos);
	}

	if(_mode == Mode::model) {
		Renderer::begin_transform(action, old_mesh);
	}

	return dragging;
}

void Gui::end_drag(Undo& undo, Scene& scene) {

	if(!dragging) return;
	
	if(_mode == Mode::scene) {
		
		Scene_Object& obj = *scene.get(selected_mesh);
		Pose p = apply_action(obj.pose);
		undo.update_obj(scene, obj.id(), p);
	
	} else if(_mode == Mode::model) {

		Pose p = apply_action({});

		if(Renderer::apply_transform(action, p)) {
			Scene_Object& obj = *scene.get(selected_mesh);
			obj.set_mesh_dirty();
			undo.update_mesh(scene, selected_mesh, std::move(old_mesh), old_id);
		}
	}

	widget_lines.clear();
	drag_start = drag_end = {};
	dragging = false;
}

void Gui::drag_to(Scene& scene, Vec3 cam, Vec2 spos, Vec3 dir) {

	if(!dragging) return;
	
	Vec3 pos;
	if(_mode == Mode::scene) {
		Scene_Object& obj = *scene.get(selected_mesh);
		pos = obj.pose.pos;
	} else {
		auto elem = Renderer::he_selected();
		assert(elem.has_value());
		pos = Halfedge_Mesh::center_of(*elem);
	}
	
	Vec3 hit; 
	Vec3 norm; norm[(int)axis] = 1.0f;

	if(action == Action::rotate) {
	
		if(!to_plane(pos, cam, dir, norm, hit)) return;

		Vec3 ang = (hit - pos).unit();
		float sgn = sign(cross(drag_start, ang)[(int)axis]);
		drag_end = {};
		drag_end[(int)axis] = sgn * Degrees(std::acos(dot(drag_start, ang)));

	} else {

		bool good;
		
		if(drag_plane) good = to_plane(pos, cam, dir, norm, hit);
		else 	       good = to_axis(pos, cam, dir, hit);

		if(!good) return;

		if(action == Action::bevel) {
			bevel_end = spos;
		} else if(action == Action::move) {
			drag_end = hit;
		} else if(action == Action::scale) {
			drag_end = {1.0f};
			drag_end[(int)axis] = (hit - pos).norm() / (drag_start - pos).norm();
		} else assert(false);
	}

	if(_mode == Mode::model) {
		if(action == Action::scale) {
			drag_end[(int)axis] *= sign(dot(hit - pos, drag_start - pos));
		}
		Pose p = apply_action({});
		if(Renderer::apply_transform(action, p)) {
			Scene_Object& obj = *scene.get(selected_mesh);
			obj.set_mesh_dirty();
		}
	}
}

bool Gui::select(Scene& scene, Undo& undo, Scene_Object::ID id, Vec3 cam, Vec2 spos, Vec3 dir) {

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
	} break;
	}

	switch(_mode) {
	case Mode::scene: return select_scene(scene, undo, id, cam, spos, dir);
	case Mode::model: return select_model(scene, undo, id, cam, spos, dir);
	default: assert(false);
	}
	return dragging;
}

void Gui::clear_select() {
	
	switch(_mode) {
	case Mode::scene: selected_mesh = 0; break;
	case Mode::model: Renderer::set_he_select(0); break;
	default: assert(false);
	}
}

bool Gui::select_model(Scene& scene, Undo& undo, Scene_Object::ID click, Vec3 cam, Vec2 spos, Vec3 dir) {

	if(click && action == Action::bevel && click == Renderer::get_he_select()) {
		
		Scene_Object& obj = *scene.get(selected_mesh);
		Halfedge_Mesh& mesh = obj.get_mesh();
		
		Halfedge_Mesh::ElementRef new_ref;
		mesh.copy_to(old_mesh);
		old_id = Renderer::get_he_select();

		auto sel = Renderer::he_selected();

		std::visit(overloaded {
			[&](Halfedge_Mesh::VertexRef vert) {
				new_ref = mesh.bevel_vertex(vert);
			},
			[&](Halfedge_Mesh::EdgeRef edge) {
				new_ref = mesh.bevel_edge(edge);
			},
			[&](Halfedge_Mesh::FaceRef face) {
				new_ref = mesh.bevel_face(face);
			},
			[&](auto) {}
		}, *sel);

		std::string err = mesh.validate();
		if(!err.empty()) {
			set_error(err);
			obj.set_mesh(old_mesh);
			old_id = Renderer::get_he_select();
		} else {
			Renderer::set_he_select(new_ref);
			dragging = true;
			drag_plane = true;
		}
	} else if(!dragging && click >= num_ids()) {
		Renderer::set_he_select((unsigned int)click);
	}

	if(dragging) {
		auto e = Renderer::he_selected();
		if(e.has_value() && !std::holds_alternative<Halfedge_Mesh::HalfedgeRef>(*e))
			return start_drag(Halfedge_Mesh::center_of(*e), cam, spos, dir);
	} 
	return dragging;
}

bool Gui::select_scene(Scene& scene, Undo& undo, Scene_Object::ID click, Vec3 cam, Vec2 spos, Vec3 dir) {

	if(dragging) {
		Scene_Object& obj = *scene.get(selected_mesh);
		return start_drag(obj.pose.pos, cam, spos, dir);
	} else if(click >= num_ids()) {
		selected_mesh = click;
	}
	return dragging;
}

void Gui::apply_transform(Scene_Object& obj) {
	if(dragging) obj.pose = apply_action(obj.pose);
}

void Gui::render_base(Mat4 viewproj) {
	Renderer::lines(baseplane, viewproj, 0.5f);
}
