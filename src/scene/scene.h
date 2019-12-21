
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
	void select(Vec2 mouse);

	void reload_shaders();
	void show_settings();

private:
	App& app;
	Camera camera;
	Mat4 view, proj, viewproj;
	GL::Shader mesh_shader, line_shader;
	GL::Framebuffer framebuffer, id_resolve;
	
	void create_baseplane();
	GL::Lines baseplane;
	
	Vec2 window_dim;
	static const int default_samples = 4;
	int samples = default_samples;
	bool settings_open = false;

	Scene_Object::ID read_id(Vec2 pos);
	float* id_buffer = nullptr;


	enum class Base_Objs : Scene_Object::ID {
		none,
		x_trans,
		y_trans,
		z_trans,
		x_rot,
		y_rot,
		z_rot,
		x_scale,
		y_scale,
		z_scale,
		count
	};
	Scene_Object x_trans, y_trans, z_trans, x_rot, y_rot, z_rot, x_scale, z_scale, y_scale;
	void render_widgets(const Scene_Object& obj);

	enum class Select_Type {
		move,
		rotate,
		scale	
	};
	Select_Type select_type = Select_Type::move;

	std::map<Scene_Object::ID, Scene_Object> objs;
	Scene_Object::ID next_id = (Scene_Object::ID)Base_Objs::count;
	Scene_Object::ID selected_id = (Scene_Object::ID)Base_Objs::none;
};

