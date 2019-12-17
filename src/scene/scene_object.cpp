
#include "scene_object.h"

Scene_Object::Scene_Object(Scene_Object&& src) :
	mesh(std::move(src.mesh)) {

	opt.name = std::move(src.opt.name);
	opt.wireframe = src.opt.wireframe; src.opt.wireframe = false;
	_id = src._id; src._id = 0;
	transform = src.transform; src.transform = Mat4::I;
}

Scene_Object::Scene_Object(ID id, Mat4 t, GL::Mesh&& m) :
	mesh(std::move(m)),
	_id(id),
	transform(t) {
	
	opt.name.reserve(max_name_len);
	snprintf(opt.name.data(), opt.name.capacity(), "Object %d", id);
}

Scene_Object::~Scene_Object() {

}

void Scene_Object::render(Mat4 view, bool outline, const GL::Shader& shader) {

	Mat4 modelview = view * transform;
	Mat4 normal = Mat4::transpose(Mat4::inverse(modelview));

	shader.uniform("modelview", modelview);
	shader.uniform("normal", normal);
	
	if(opt.wireframe) {
		shader.uniform("solid", true);
		shader.uniform("color", Vec3());
		GL::begin_wireframe();
		mesh.render();
		GL::end_wireframe();
	}

	shader.uniform("modelview", modelview);
	shader.uniform("normal", normal);
	shader.uniform("solid", false);
	shader.uniform("color", color);

	if(outline) GL::start_stencil();

	mesh.render();

	if(outline) {
		Mat4 s_mv = modelview * Mat4::scale(Vec3(1.02f, 1.02f, 1.02f));
		shader.uniform("modelview", s_mv);
		shader.uniform("solid", true);
		shader.uniform("color", Vec3(242.0f / 255.0f, 153.0f / 255.0f, 41.0f / 255.0f));

		GL::use_stencil();
		mesh.render();
		GL::end_stencil();
	}
}
