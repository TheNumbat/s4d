
#pragma once

#include "../lib/math.h"
#include "../lib/gl.h"
#include "../lib/camera.h"

#include "scene_object.h"

#include <map>

class App;
class Scene {
public:
	Scene(Vec2 window_dim, App& app);
	~Scene();

	void render();
	void gui();

	void apply_window_dim(Vec2 new_dim);
	void add_object(Scene_Object&& obj);

	void camera_orbit(Vec2 dmouse);
	void camera_move(Vec2 dmouse);
	void camera_radius(float dmouse);
	
	bool select(Vec2 mouse);
	void drag(Vec2 mouse);
	void end_drag(Vec2 mouse);

	void reload_shaders();
	void show_settings();

private:
	App& app;

	// Camera 
	Camera camera;
	Mat4 view, proj, viewproj, iviewproj;
	
	// GL data
	static const int default_samples = 4;
	int samples = default_samples;
	GL::Shader mesh_shader, line_shader;
	GL::Framebuffer framebuffer, id_resolve;
	
	// Baseplane
	void create_baseplane();
	GL::Lines baseplane;
	
	// Picking
	Scene_Object::ID read_id(Vec2 pos);
	unsigned char* id_buffer = nullptr;

	// GUI
	struct Gui {
		enum class Axis {
			X, Y, Z
		};
		enum class Action {
			move, rotate, scale
		};
		enum class Basic : Scene_Object::ID {
			none,
			x_trans, y_trans, z_trans,
			x_rot, y_rot, z_rot,
			x_scale, y_scale, z_scale,
			count
		};
		Vec2 window_dim;
		bool settings_open = false;
		
		bool dragging = false;
		Action action = Action::move;
		Axis axis = Axis::X;
		Vec3 offset, scale = {1.0f};
		Scene_Object::ID id = (Scene_Object::ID)Basic::none;

		Scene_Object x_trans, y_trans, z_trans, x_rot, y_rot, z_rot, x_scale, z_scale, y_scale;

		// NOTE(max): copied from blender
		static inline const Vec3 outline = Vec3(242.0f / 255.0f, 153.0f / 255.0f, 41.0f / 255.0f);
		static inline const Vec3 baseplane = Vec3(71.0f / 255.0f);
		static inline const Vec3 background = Vec3(58.0f / 255.0f);
		static inline const Vec3 red = Vec3(163.0f / 255.0f, 66.0f / 255.0f, 81.0f / 255.0f);
		static inline const Vec3 green = Vec3(124.0f / 255.0f, 172.0f / 255.0f, 40.0f / 255.0f);
		static inline const Vec3 blue = Vec3(64.0f / 255.0f, 127.0f / 255.0f, 193.0f / 255.0f);
	};
	Gui state;
	void render_selected(Scene_Object& obj);
	Vec3 screen_to_world(Vec2 mouse);
	bool screen_to_axis(Scene_Object& obj, Vec2 mouse, Vec3& hit);

	// User Objects
	std::map<Scene_Object::ID, Scene_Object> objs;
	Scene_Object::ID next_id = (Scene_Object::ID)Gui::Basic::count;
};

