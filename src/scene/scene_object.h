
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

	void render(Mat4 view, const GL::Shader& shader);

	ID id() {return _id;}
	std::string name;

private:
	static const int max_name_len = 256;
	
	Vec3 color = {0.7f, 0.7f, 0.7f};
	ID _id = 0;
	Mat4 transform;
	GL::Mesh mesh;
};
