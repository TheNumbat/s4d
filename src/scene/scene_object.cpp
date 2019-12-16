
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

void Scene_Object::render(const GL_Shader& shader) {

    glUniformMatrix4fv(shader.uniform("model"), 1, GL_FALSE, transform.data);
    glUniform3fv(shader.uniform("color"), 1, color.data);
    mesh.render();
}
