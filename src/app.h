
#pragma once

#include <map>
#include <string>
#include <SDL2/SDL.h>

#include "lib/math.h"
#include "lib/camera.h"
#include "platform/gl.h"
#include "scene/scene.h"

#include "gui.h"

class Platform;
class App {
public:
	App(Platform& plt);
	~App();

	void render();
	void event(SDL_Event e);
	void settings();

private:
	Scene_Object::ID read_id(Vec2 pos);
	void apply_window_dim(Vec2 new_dim);
	void render_selected(Scene_Object& obj);
	Vec3 screen_to_world(Vec2 mouse);

	// Systems
	Platform& plt;
	Scene scene;
	Gui gui;
	bool gui_capture = false;

	// Camera data
    enum class Camera_Control {
        none,
        orbit,
        move
    };
	Camera_Control cam_mode = Camera_Control::none;
	Camera camera;
	Mat4 view, proj, viewproj, iviewproj;
	Vec2 window_dim, last_mouse;

	// GL data
	int samples = 4;
	bool settings_open = false;
	GL::Shader mesh_shader, line_shader;
	GL::Framebuffer framebuffer, id_resolve;
	unsigned char* id_buffer = nullptr;
};
