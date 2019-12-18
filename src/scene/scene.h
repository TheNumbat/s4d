
#pragma once

#include "../lib/math.h"
#include "../lib/gl.h"
#include "../lib/camera.h"

#include "scene_object.h"

#include <map>

class Scene {
public:
	Scene(Vec2 window_dim);
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
	Camera camera;
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

	std::map<Scene_Object::ID, Scene_Object> objs;
	Scene_Object::ID next_id = 1;
	Scene_Object::ID selected_id = 0;
};

