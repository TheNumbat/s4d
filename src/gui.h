
#pragma once

#include "lib/math.h"
#include <SDL2/SDL.h>

#include "scene/scene.h"
#include "undo.h"

// GUI
class Gui {
public:
	enum class Mode {
		scene,
		model,
		// render,
		// rig,
		// animate,
		// simulate
	};
	enum class Axis {
		X, Y, Z
	};
	struct Color {
		static inline const Vec3 outline = Vec3(242.0f, 153.0f, 41.0f) / 255.0f;
		static inline const Vec3 baseplane = Vec3(71.0f) / 255.0f;
		static inline const Vec3 background = Vec3(58.0f) / 255.0f;
		static inline const Vec3 red = Vec3(163.0f, 66.0f, 81.0f) / 255.0f;
		static inline const Vec3 green = Vec3(124.0f, 172.0f, 40.0f) / 255.0f;
		static inline const Vec3 blue = Vec3(64.0f, 127.0f, 193.0f) / 255.0f;
		static Vec3 axis(Axis a);
	};
	static Scene_Object::ID num_ids();
	
	Gui(Vec2 window_dim);
	~Gui();

	Mode mode() const {return _mode;}

	// Input
	void update_dim(Vec2 dim);
	void set_error(std::string msg);
	bool keydown(Undo& undo, Scene& scene, SDL_Keycode key);

	// Object interaction
	bool select(Scene& scene, Scene_Object::ID id, Vec3 cam, Vec3 dir);
	void clear_select();

	void drag_to(Scene& scene, Vec3 cam, Vec3 dir);
	void end_drag(Undo& undo, Scene& scene);
	void apply_transform(Scene_Object& obj);
	Scene_Object::ID selected_id();

	// 2D GUI rendering
	float menu(Scene& scene, Undo& undo, bool& settings);
	void error();
	void objs(Scene& scene, Undo& undo, float menu_height);

	// 3D GUI rendering
	void render_widgets(Mat4 viewproj, Mat4 view, const Pose& pose, float scale);
	void render_base(Mat4 viewproj);

private:
	static inline const char* file_types = "dae,obj,fbx,glb,gltf,3ds,blend";
	void load_scene(Scene& scene, Undo& undo);
	void write_scene(Scene& scene);

	Vec3 apply_action(const Scene_Object& obj);
	bool to_axis(const Scene_Object& obj, Vec3 pos, Vec3 dir, Vec3& hit);
	bool to_plane(const Scene_Object& obj, Vec3 pos, Vec3 dir, Vec3& hit);

	bool select_scene(Scene& scene, Scene_Object::ID click, Vec3 cam, Vec3 dir);
	bool select_model(Scene& scene, Scene_Object::ID click, Vec3 cam, Vec3 dir);

	// Error handling
	bool error_shown = false;
	std::string error_msg;

	// Edit mode
	Mode _mode = Mode::scene;

	// Object transform actions
	enum class Action {
		move, rotate, scale
	};
	Axis axis = Axis::X;
	Action action = Action::move;
	bool dragging = false, drag_plane = false;
	Vec3 drag_start, drag_end;
	Vec3 gui_drag_start;

	// 3D GUI Objects
	enum class Basic : Scene_Object::ID {
		none,
		x_trans, y_trans, z_trans,
		xy_trans, yz_trans, xz_trans,
		x_rot, y_rot, z_rot,
		x_scale, y_scale, z_scale,
		count
	};
	GL::Lines baseplane, widget_lines;
	void create_baseplane();
	void generate_widget_lines(const Scene_Object& obj);
	Scene_Object::ID selected_mesh = (Scene_Object::ID)Basic::none;
	Scene_Object::ID selected_compo = (Scene_Object::ID)Basic::none;
	Scene_Object x_trans, y_trans, z_trans, x_rot, y_rot, z_rot;
	Scene_Object x_scale, z_scale, y_scale, xy_trans, yz_trans, xz_trans;
	
	// Screen
	Vec2 window_dim;
};
