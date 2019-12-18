
#include "scene_object.h"

Scene_Object::Scene_Object(Scene_Object&& src) :
	mesh(std::move(src.mesh)) {

	opt.name = std::move(src.opt.name);
	opt.wireframe = src.opt.wireframe; src.opt.wireframe = false;
	_id = src._id; src._id = 0;
	transform = src.transform; src.transform = Mat4::I;
}

Scene_Object::Scene_Object(ID id, Mat4 t, GL::Mesh&& m) :
	_id(id),
	transform(t),
	mesh(std::move(m)) {
	
	opt.name.reserve(max_name_len);
	snprintf(opt.name.data(), opt.name.capacity(), "Object %d", id);
}

Scene_Object::~Scene_Object() {

}

void Scene_Object::render(Mat4 view, bool outline, const GL::Shader& shader) {

	Mat4 modelview = view * transform;
	Mat4 normal = Mat4::transpose(Mat4::inverse(modelview));

	shader.uniform("id", _id);
	shader.uniform("modelview", modelview);
	shader.uniform("normal", normal);
	
	if(opt.wireframe) {
		shader.uniform("scale", 0.0f);
		shader.uniform("solid", true);
		shader.uniform("front", false);
		shader.uniform("write_id", false);
		shader.uniform("color", Vec3());
		GL::begin_wireframe();
		mesh.render();
		GL::end_wireframe();
	}

	if(outline) GL::start_stencil();

	shader.uniform("scale", 0.0f);
	shader.uniform("solid", false);
	shader.uniform("front", false);
	shader.uniform("color", color);
	shader.uniform("write_id", true);
	mesh.render();

	if(outline) {

		shader.uniform("scale", 0.02f);
		shader.uniform("solid", true);
		shader.uniform("front", true);
		shader.uniform("write_id", false);
		shader.uniform("color", outline_color);

		GL::use_stencil();
		mesh.render();
		GL::end_stencil();
	}
}
