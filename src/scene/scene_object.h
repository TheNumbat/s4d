
#pragma once

#include "../lib/math.h"
#include "../lib/gl.h"

struct Pose {
	Vec3 pos;
	Vec3 euler;
	Vec3 scl = {1.0f, 1.0f, 1.0f};
	
	Mat4 transform() const;

	static Pose rotate(Vec3 angles);
	static Pose move(Vec3 t);
	static Pose scale(Vec3 s);
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

	void render(Mat4 view, const GL::Shader& shader, bool solid = false) const;

	ID id() {return _id;}
	
	struct Options {
		std::string name;
		bool wireframe = false;
	};
	Options opt;
	Pose pose;

private:
	static const int max_name_len = 256;
	static inline const Vec3 outline_color = Vec3(242.0f / 255.0f, 153.0f / 255.0f, 41.0f / 255.0f);

	Vec3 color;
	ID _id = 0;
	GL::Mesh mesh;
};
