
#pragma once

#include "../lib/math.h"
#include "../lib/gl.h"

class Scene_Object {
public:
	typedef int ID;

	Scene_Object(ID id, Mat4 transform, GL::Mesh&& mesh);
	Scene_Object(const Scene_Object& src) = delete;
	Scene_Object(Scene_Object&& src);
	~Scene_Object();

	void render(Mat4 view, bool outline, const GL::Shader& shader);

	ID id() {return _id;}
	
	struct Options {
		std::string name;
		bool wireframe = false;
	};
	Options opt;

private:
	static const int max_name_len = 256;
	static inline const Vec3 outline_color = Vec3(242.0f / 255.0f, 153.0f / 255.0f, 41.0f / 255.0f);

	Vec3 color = {0.7f, 0.7f, 0.7f};
	ID _id = 0;
	Mat4 transform;
	GL::Mesh mesh;
};
