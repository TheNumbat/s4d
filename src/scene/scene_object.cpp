
#include "scene_object.h"

Mat4 Pose::transform() const {
	return Mat4::translate(pos) * 
		   Mat4::rotate(euler.y, {0.0f, 1.0f, 0.0f}) * 
		   Mat4::rotate(euler.z, {0.0f, 0.0f, 1.0f}) * 
		   Mat4::rotate(euler.x, {1.0f, 0.0f, 0.0f}) * 
		   Mat4::scale(scl);
}

Pose Pose::rotate(Vec3 angles) {
	return {{}, angles, {1.0f, 1.0f, 1.0f}};
}

Pose Pose::move(Vec3 t) {
	return {t, {}, {1.0f, 1.0f, 1.0f}};
}

Pose Pose::scale(Vec3 s) {
	return {{}, {}, s};
}

Scene_Object::Scene_Object() {

}

Scene_Object::Scene_Object(Scene_Object&& src) :
	mesh(std::move(src.mesh)) {

	opt.name = std::move(src.opt.name);
	opt.wireframe = src.opt.wireframe; src.opt.wireframe = false;
	_id = src._id; src._id = 0;
	color = src.color; src.color = {};
	pose = src.pose; src.pose = {};
}

Scene_Object::Scene_Object(ID id, Pose p, GL::Mesh&& m, Vec3 c) :
	_id(id),
	pose(p),
	mesh(std::move(m)),
	color(c) {
	
	opt.name.reserve(max_name_len);
	snprintf(opt.name.data(), opt.name.capacity(), "Object %d", id);
}

Scene_Object::~Scene_Object() {

}

void Scene_Object::operator=(Scene_Object&& src) {
	mesh = std::move(src.mesh);
	opt.name = std::move(src.opt.name);
	opt.wireframe = src.opt.wireframe; src.opt.wireframe = false;
	_id = src._id; src._id = 0;
	color = src.color; src.color = {};
	pose = src.pose; src.pose = {};
}

void Scene_Object::render(Mat4 view, const GL::Shader& shader, bool solid, bool depth_only) const {

	Mat4 modelview = view * pose.transform();
	Mat4 normal = Mat4::transpose(Mat4::inverse(modelview));

	shader.uniform("id", _id);
	shader.uniform("modelview", modelview);
	shader.uniform("normal", normal);
	
	if(depth_only) GL::color_mask(false);

	if(opt.wireframe) {
		shader.uniform("scale", 0.0f);
		shader.uniform("solid", solid);
		shader.uniform("write_id", false);
		shader.uniform("color", Vec3());
		GL::begin_wireframe();
		mesh.render();
		GL::end_wireframe();
	}

	shader.uniform("scale", 0.0f);
	shader.uniform("solid", solid);
	shader.uniform("color", color);
	shader.uniform("write_id", true);
	mesh.render();

	if(depth_only) GL::color_mask(true);
}
