
#pragma once

#include "../lib/math.h"
#include "../platform/gl.h"
#include "halfedge.h"

#include <map>
#include <optional>
#include <functional>

#include <assimp/scene.h>

class Undo;

struct Pose {
	Vec3 pos;
	Vec3 euler;
	Vec3 scale = {1.0f};

	Mat4 transform() const;
	Mat4 rotation_mat() const;
	Quat rotation_quat() const;

	void clamp_euler();
	bool valid() const;

	static Pose rotated(Vec3 angles);
	static Pose moved(Vec3 t);
	static Pose scaled(Vec3 s);
};

class Scene_Object {
public:
	using ID = unsigned int;

	Scene_Object();
	Scene_Object(ID id, Pose pose, GL::Mesh&& mesh, Vec3 color = {0.7f, 0.7f, 0.7f});
	Scene_Object(ID id, Pose pose, Halfedge_Mesh&& mesh, Vec3 color = {0.7f, 0.7f, 0.7f});
	Scene_Object(const Scene_Object& src) = delete;
	Scene_Object(Scene_Object&& src);
	~Scene_Object();

	void operator=(const Scene_Object& src) = delete;
	void operator=(Scene_Object&& src);

	void sync_mesh();
	void render_mesh(Mat4 view, bool solid = false, bool depth_only = false);
	void render_halfedge(Mat4 view);

	ID id() const {return _id;}
	const GL::Mesh& mesh() const {return _mesh;}
	
	BBox bbox() const;
	
	struct Options {
		std::string name;
		bool wireframe = false;
	};
	Options opt;
	Pose pose;

private:
	static const int max_name_len = 256;

	Vec3 color;
	ID _id = 0;
	bool editable = true;
	Halfedge_Mesh halfedge;
	
	GL::Mesh _mesh;
	bool mesh_dirty = false;
};

class Scene {
public:
    Scene(Scene_Object::ID start);
    ~Scene();

	std::string write(std::string file);
	std::string load(bool clear_first, Undo& undo, std::string file);
	void clear(Undo& undo);

    bool empty();
    size_t size();
	Scene_Object::ID add(Scene_Object&& obj);
    Scene_Object::ID add(Pose pose, GL::Mesh&& mesh, Scene_Object::ID id = 0);
	Scene_Object::ID reserve_id();
    
	void erase(Scene_Object::ID id);
	void restore(Scene_Object::ID id);

    void render_objs(Mat4 view, Scene_Object::ID selected);
    void for_objs(std::function<void(Scene_Object&)> func);

    std::optional<std::reference_wrapper<Scene_Object>> get(Scene_Object::ID id);

private:
	void load_node(std::vector<std::string>& errors, const aiScene* scene, aiNode* node, aiMatrix4x4 transform);

	std::map<Scene_Object::ID, Scene_Object> objs;
	std::map<Scene_Object::ID, Scene_Object> erased;
	Scene_Object::ID next_id, first_id;
};
