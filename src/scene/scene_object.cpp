
#include "scene_object.h"

Scene_Object::Scene_Object(Scene_Object&& src) :
    mesh(std::move(src.mesh)), 
    _id(src._id), 
    transform(src.transform) {
}

Scene_Object::Scene_Object(ID id, Mat4 t, GL_Mesh&& m) :
    mesh(std::move(m)),
    _id(id),
    transform(t) {
}

Scene_Object::~Scene_Object() {

}

void Scene_Object::render(Mat4 view, const GL_Shader& shader) {

    Mat4 modelview = view * transform;
    Mat4 normal = Mat4::transpose(Mat4::inverse(modelview));

    glUniformMatrix4fv(shader.uniform("modelview"), 1, GL_FALSE, modelview.data);
    glUniformMatrix4fv(shader.uniform("normal"), 1, GL_FALSE, normal.data);
    glUniform3fv(shader.uniform("color"), 1, color.data);
    mesh.render();
}
