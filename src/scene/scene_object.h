
#pragma once

#include "../lib/math.h"
#include "../lib/gl.h"

struct Pose {
	Vec3 pos;
	Vec3 euler;
	Vec3 scale = {1.0f, 1.0f, 1.0f};
	
	Mat4 transform() const;
	Mat4 transform_rotate() const;
	void clamp_euler();
	Vec3 internal_axis(Vec3 axis) const;
	Vec3 external_axis(Vec3 axis) const;
	bool valid() const;

	static Pose rotated(Vec3 angles);
	static Pose moved(Vec3 t);
	static Pose scaled(Vec3 s);
};

class Scene_Object {
public:
	typedef int ID;

	Scene_Object();
	Scene_Object(ID id, Pose pose, GL::Mesh&& mesh, Vec3 color = {0.7f, 0.7f, 0.7f});
	Scene_Object(const Scene_Object& src) = delete;
	Scene_Object(Scene_Object&& src);
	~Scene_Object();

	void operator=(const Scene_Object& src) = delete;
	void operator=(Scene_Object&& src);

	void render(Mat4 view, const GL::Shader& shader, bool solid = false, bool depth_only = false) const;

	ID id() const {return _id;}
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
	GL::Mesh mesh;
};
