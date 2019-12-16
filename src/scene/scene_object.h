
#pragma once

#include "../lib/math.h"
#include "../lib/gl.h"

class Scene_Object {
public:
    typedef int ID;

    Scene_Object(ID id, Mat4 transform, GL_Mesh&& mesh);
    Scene_Object(const Scene_Object& src) = delete;
	Scene_Object(Scene_Object&& src);
    ~Scene_Object();

    void render(const GL_Shader& shader);

    ID id() {return _id;}

private:
    Vec3 color = {0.7f, 0.7f, 0.7f};
    ID _id = 0;
    Mat4 transform;
    GL_Mesh mesh;
};
