
#include "app.h"
#include "scene/render.h"
#include "scene/util.h"
#include "platform/platform.h"

#include <SDL2/SDL.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>

App::App(Platform& plt) : 
	window_dim(plt.window_draw()),
	camera(window_dim),
	plt(plt),
	scene(Gui::num_ids()),
	gui(window_dim) {

	GL::global_params();
	Renderer::setup(window_dim);
}

App::~App() {
	Renderer::shutdown();
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

			apply_window_dim(plt.window_draw());
		}
	} break;

	case SDL_MOUSEMOTION: {

		Vec2 d(e.motion.xrel, e.motion.yrel);
		Vec2 p = plt.scale_mouse({e.motion.x, e.motion.y});
		
		if(gui_capture) {
			gui.drag_to(scene, camera.pos(), screen_to_world(p));
		} else if(cam_mode == Camera_Control::orbit) {
			camera.mouse_orbit(d);
		} else if(cam_mode == Camera_Control::move) {
			camera.mouse_move(d);
		}

	} break;

	case SDL_MOUSEBUTTONDOWN: {

		if(IO.WantCaptureMouse) break;

		Vec2 p = plt.scale_mouse({e.button.x, e.button.y});

		if(e.button.button == SDL_BUTTON_LEFT) {

			Scene_Object::ID id = Renderer::read_id(p);
			if(gui.select(scene, id, camera.pos(), screen_to_world(p))) {
				cam_mode = Camera_Control::none;
				plt.grab_mouse();
				gui_capture = true;
			} else if(cam_mode == Camera_Control::none) {
				cam_mode = Camera_Control::orbit;
			}
		} else if(e.button.button == SDL_BUTTON_RIGHT) {
			if(cam_mode == Camera_Control::none) {
				cam_mode = Camera_Control::move;
			}
		}
	} break;

	case SDL_MOUSEBUTTONUP: {

		Vec2 p = plt.scale_mouse({e.button.x, e.button.y});

		if(e.button.button == SDL_BUTTON_LEFT) {
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

void App::render_selected(Scene_Object& obj) {

	Vec3 prev_scale = obj.pose.scale;
	Vec3 prev_rot = obj.pose.euler;
	Vec3 prev_pos = obj.pose.pos;

	if(gui.mode() == Gui::Mode::scene) {
		
		gui.apply_transform(obj);
		obj.render_mesh(view);
		Renderer::outline(viewproj, view, obj);

	} else if(gui.mode() == Gui::Mode::model) {
		
		obj.render_halfedge(view);

	} else assert(false);

	float scl = (camera.pos() - obj.pose.pos).norm() / 5.5f;
	gui.render_widgets(viewproj, view, obj.pose, scl);

	obj.pose.scale = prev_scale;
	obj.pose.euler = prev_rot;
	obj.pose.pos = prev_pos;
}

void App::render() {

	proj = camera.proj();
	view = camera.view();	
	viewproj = proj * view;
	iviewproj = Mat4::inverse(viewproj);

	Renderer::begin();
	if(gui.mode() == Gui::Mode::scene) {
		Renderer::proj(proj);
        scene.render_objs(view, gui.selected_id());
	}
	gui.render_base(viewproj);

	auto selected = scene.get(gui.selected_id());
	if(selected.has_value()) {
		render_selected(*selected);
	}
	Renderer::complete();

	// GUI
	float height = gui.menu(scene, undo, settings_open);
	gui.objs(scene, undo, height);
	gui.error();
	if(settings_open) Renderer::settings_gui(&settings_open);
}

Vec3 App::screen_to_world(Vec2 mouse) {

	Vec2 t(2.0f * mouse.x / window_dim.x - 1.0f, 
		   1.0f - 2.0f * mouse.y / window_dim.y);
	Vec3 p = iviewproj * Vec3(t.x, t.y, 0.1f);
	return (p - camera.pos()).unit();
}

void App::apply_window_dim(Vec2 new_dim) {

	window_dim = new_dim;

	camera.set_ar(window_dim);
	gui.update_dim(window_dim);
	Renderer::update_dim(window_dim);
	GL::viewport(window_dim);
}
