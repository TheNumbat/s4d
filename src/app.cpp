
#include "app.h"
#include "scene/util.h"
#include "platform/platform.h"

#include <SDL2/SDL.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>

App::App(Platform& plt) : 
	window_dim(plt.window_dim()),
	camera(window_dim),
	plt(plt),
	scene(Gui::num_ids()),
	gui(window_dim),
	mesh_shader(GL::Shaders::mesh_v, GL::Shaders::mesh_f),
	line_shader(GL::Shaders::line_v, GL::Shaders::line_f),
	framebuffer(2, window_dim, samples),
	id_resolve(1, window_dim, 1, false) {

	id_buffer = new GLubyte[(int)window_dim.x * (int)window_dim.y * 4];
	GL::global_params();
}

App::~App() {
	delete[] id_buffer;
}

void App::event(SDL_Event e) {

	ImGuiIO& IO = ImGui::GetIO();

	switch(e.type) {
	case SDL_KEYDOWN: {
		if(IO.WantCaptureKeyboard) break;
		if(gui.keydown(undo, scene, e.key.keysym.sym)) break;

		if(e.key.keysym.sym == SDLK_z) {
			if(e.key.keysym.mod & KMOD_CTRL) {
				undo.undo();
			}
		}
		else if(e.key.keysym.sym == SDLK_y) {
			if(e.key.keysym.mod & KMOD_CTRL) {
				undo.redo();
			}
		}
	} break;

	case SDL_WINDOWEVENT: {
		if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
			e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {

			apply_window_dim(plt.window_dim());
		}
	} break;

	case SDL_MOUSEMOTION: {

		Vec2 d(e.motion.xrel, e.motion.yrel);
		Vec2 p(e.motion.x, e.motion.y);
		
		if(gui_capture) {
			gui.drag_to(scene, camera.pos(), screen_to_world(p));
		} else if(cam_mode == Camera_Control::orbit) {
			camera.mouse_orbit(d);
		} else if(cam_mode == Camera_Control::move) {
			if(gui.mode() == Gui::Mode::scene)
				camera.mouse_move(d);
		}

	} break;

	case SDL_MOUSEBUTTONDOWN: {

		if(IO.WantCaptureMouse) break;

		Vec2 p(e.button.x, e.button.y);

		if(e.button.button == SDL_BUTTON_RIGHT) {

			Scene_Object::ID id = read_id(p);
			if(gui.select(scene, id, camera.pos(), screen_to_world(p))) {
				cam_mode = Camera_Control::none;
				plt.grab_mouse();
				gui_capture = true;
			} else if(cam_mode == Camera_Control::none) {
				cam_mode = Camera_Control::move;
			}
		} else if(e.button.button == SDL_BUTTON_LEFT) {
			if(cam_mode == Camera_Control::none) {
				cam_mode = Camera_Control::orbit;
			}
		}
	} break;

	case SDL_MOUSEBUTTONUP: {

		Vec2 p(e.button.x, e.button.y);

		if(e.button.button == SDL_BUTTON_RIGHT) {
			if(!IO.WantCaptureMouse && gui_capture) {
				gui_capture = false;
				gui.drag_to(scene, camera.pos(), screen_to_world(p));
				gui.end_drag(undo, scene);
				plt.ungrab_mouse();
				break;
			}
		}

		if((e.button.button == SDL_BUTTON_LEFT && cam_mode == Camera_Control::orbit) ||
		   (e.button.button == SDL_BUTTON_RIGHT && cam_mode == Camera_Control::move)) {
			cam_mode = Camera_Control::none;
		}

	} break;

	case SDL_MOUSEWHEEL: {
		camera.mouse_radius((float)e.wheel.y);
	} break;
	}
}

Scene_Object::ID App::read_id(Vec2 pos) {
	
	int x = (int)pos.x;
	int y = (int)(window_dim.y - pos.y - 1);

	if(id_resolve.can_read_at()) {
		
		GLubyte data[4] = {};
		id_resolve.read_at(0, x, y, data);
		return (int)data[0] | (int)data[1] << 8 | (int)data[2] << 16;

	} else {
		int idx = y * (int)window_dim.x * 4 + x * 4;
	
		int a = id_buffer[idx];
		int b = id_buffer[idx + 1];
		int c = id_buffer[idx + 2];

		return a | b << 8 | c << 16;
	}
	return 0;
}

void App::render_selected(Scene_Object& obj) {

	Vec3 prev_scale = obj.pose.scale;
	Vec3 prev_rot = obj.pose.euler;
	Vec3 prev_pos = obj.pose.pos;

	if(gui.mode() == Gui::Mode::scene) {
		
		gui.apply_transform(obj);
		mesh_shader.bind();
		obj.render_mesh(view, mesh_shader);

		framebuffer.clear_d();
		obj.render_mesh(view, mesh_shader, false, true);

		Vec2 min, max;
		obj.bbox().screen_rect(viewproj, min, max);

		GL::flush_if_nvidia();
		GL::Effects::outline(framebuffer, framebuffer, Gui::Color::outline, 
						 	min - Vec2(3.0f / window_dim.x), 
						 	max + Vec2(3.0f / window_dim.y));
		GL::flush_if_nvidia();

	} else if(gui.mode() == Gui::Mode::model) {
		
		obj.pose = {};

		mesh_shader.bind();
		obj.render_halfedge(view, mesh_shader);

	} else assert(false);

	framebuffer.clear_d();

	float scl = (camera.pos() - obj.pose.pos).norm() / 5.5f;
	gui.render_widgets(view, line_shader, mesh_shader, obj.pose, scl);

	obj.pose.scale = prev_scale;
	obj.pose.euler = prev_rot;
	obj.pose.pos = prev_pos;
}

void App::render() {

	proj = camera.proj();
	
	if(gui.mode() == Gui::Mode::scene) {
		view = camera.view();
	} else if(gui.mode() == Gui::Mode::model) {
		view = camera.view_origin();
	} else assert(false);

	viewproj = proj * view;
	iviewproj = Mat4::inverse(viewproj);

	framebuffer.clear(0, Vec4(Gui::Color::background, 1.0f));
	framebuffer.clear(1, {0.0f, 0.0f, 0.0f, 1.0f});
	framebuffer.clear_d();
	framebuffer.bind();
	if(gui.mode() == Gui::Mode::scene) {
		mesh_shader.bind();
		mesh_shader.uniform("proj", proj);

        scene.render_objs(view, mesh_shader, gui.selected_id());
	}
	{
		line_shader.bind();
		line_shader.uniform("alpha", 1.0f);
		line_shader.uniform("viewproj", viewproj);
		gui.render_base(framebuffer.is_multisampled());
	}

	auto selected = scene.get(gui.selected_id());
	if(selected.has_value()) {
		render_selected(*selected);
	}

	framebuffer.blit_to(1, id_resolve, false);
	
	if(!id_resolve.can_read_at())
		id_resolve.read(0, id_buffer);

	framebuffer.blit_to_screen(0, window_dim);

	// GUI
	float height = gui.menu(scene, undo, settings_open);
	gui.objs(scene, undo, height);
	gui.error();
	settings();
}

Vec3 App::screen_to_world(Vec2 mouse) {

	Vec2 t(2.0f * mouse.x / window_dim.x - 1.0f, 
		   1.0f - 2.0f * mouse.y / window_dim.y);
	Vec3 p = iviewproj * Vec3(t.x, t.y, 0.1f);
	return (p - camera.pos()).unit();
}

void App::apply_window_dim(Vec2 new_dim) {

	window_dim = new_dim;

	delete[] id_buffer;
	id_buffer = new GLubyte[(int)window_dim.x * (int)window_dim.y * 4]();

	camera.set_ar(window_dim);
	gui.update_dim(window_dim);
	framebuffer.resize(window_dim, samples);
	id_resolve.resize(window_dim);
	GL::viewport(window_dim);
}

void App::settings() {
	if(settings_open) {
		ImGui::Begin("Display Settings", &settings_open, 
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
		
		ImGui::InputInt("Multisampling", &samples);
		if(samples < 1) samples = 1;
		if(samples > 16) samples = 16;

		if(ImGui::Button("Apply")) {
			framebuffer.resize(window_dim, samples);
		}

		ImGui::Separator();
		ImGui::Text("GPU: %s", GL::renderer().c_str());
		ImGui::Text("OpenGL: %s", GL::version().c_str());

		ImGui::End();
	}
}

