
#include "scene_object.h"

Scene_Object::Scene_Object(Scene_Object&& src) :
	mesh(std::move(src.mesh)), 
	name(std::move(src.name)),
	_id(src._id), 
	transform(src.transform) {
}

Scene_Object::Scene_Object(ID id, Mat4 t, GL::Mesh&& m) :
	mesh(std::move(m)),
	_id(id),
	transform(t) {
	
	name.reserve(max_name_len);
	snprintf(name.data(), name.capacity(), "Object %d", id);
}

Scene_Object::~Scene_Object() {

}

void Scene_Object::render(Mat4 view, const GL::Shader& shader) {

	Mat4 modelview = view * transform;
	Mat4 normal = Mat4::transpose(Mat4::inverse(modelview));

	shader.uniform("modelview", modelview);
	shader.uniform("normal", normal);
	shader.uniform("color", color);
	mesh.render();
}
