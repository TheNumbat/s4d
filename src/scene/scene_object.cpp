
#include "scene_object.h"

Mat4 Pose::transform() const {
	return Mat4::translate(pos) * 
		   rotation() *
		   Mat4::scale(scale);
}

Mat4 Pose::rotation() const {
	return Mat4::rotate(euler.z, {0.0f, 0.0f, 1.0f}) *
		   Mat4::rotate(euler.y, {0.0f, 1.0f, 0.0f}) *  
		   Mat4::rotate(euler.x, {1.0f, 0.0f, 0.0f});
}

bool Pose::valid() const {
	return pos.valid() && euler.valid() && scale.valid();
}

void Pose::clamp_euler() {
	if(!valid()) return;
	while(euler.x < 0) euler.x += 360.0f;
	while(euler.x >= 360.0f) euler.x -= 360.0f;
	while(euler.y < 0) euler.y += 360.0f;
	while(euler.y >= 360.0f) euler.y -= 360.0f;
	while(euler.z < 0) euler.z += 360.0f;
	while(euler.z >= 360.0f) euler.z -= 360.0f;
}

Pose Pose::rotated(Vec3 angles) {
	return {{}, angles, {1.0f, 1.0f, 1.0f}};
}

Pose Pose::moved(Vec3 t) {
	return {t, {}, {1.0f, 1.0f, 1.0f}};
}

Pose Pose::scaled(Vec3 s) {
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
	pose(p),
	color(c),
	_id(id),
	mesh(std::move(m)) {
	
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

BBox Scene_Object::bbox() const {

	Mat4 t = pose.transform();
	BBox ret;
	std::vector<Vec3> c = mesh.bbox().corners();
	for(auto& v : c) ret.enclose(t * v);
	return ret;
}

void Scene_Object::render(Mat4 view, const GL::Shader& shader, bool solid, bool depth_only) const {

	Mat4 modelview = view * pose.transform();
	Mat4 normal = Mat4::transpose(Mat4::inverse(modelview));

	shader.uniform("id", _id);
	shader.uniform("modelview", modelview);
	shader.uniform("normal", normal);
	shader.uniform("solid", solid);
	
	if(depth_only) GL::color_mask(false);

	if(opt.wireframe) {
		shader.uniform("color", Vec3());
		GL::enable(GL::Opt::wireframe);
		mesh.render();
		GL::disable(GL::Opt::wireframe);
	}

	shader.uniform("color", color);
	mesh.render();

	if(depth_only) GL::color_mask(true);
}
