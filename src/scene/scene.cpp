
#include "scene.h"
#include "../lib/log.h"
#include "../undo.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Mat4 Pose::transform() const {
	return Mat4::translate(pos) * 
		   rotation_mat() *
		   Mat4::scale(scale);
}

Mat4 Pose::rotation_mat() const {
	return Mat4::rotate(euler.z, {0.0f, 0.0f, 1.0f}) *
		   Mat4::rotate(euler.y, {0.0f, 1.0f, 0.0f}) *  
		   Mat4::rotate(euler.x, {1.0f, 0.0f, 0.0f});
}

Quat Pose::rotation_quat() const {
	return Quat::euler(euler);
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

Scene::Scene(Scene_Object::ID start) :
	next_id(start),
	first_id(start) {
}

Scene::~Scene() {

}

Scene_Object::ID Scene::get_id() {
	return next_id++;
}

Scene_Object::ID Scene::add(Pose pose, GL::Mesh&& mesh, Scene_Object::ID id) {
	if(!id) id = next_id++;
	assert(objs.find(id) == objs.end());
	objs.emplace(std::make_pair(id, Scene_Object(id, pose, std::move(mesh))));
	return id;
}

void Scene::restore(Scene_Object::ID id) {
	if(objs.find(id) != objs.end()) return;
	assert(erased.find(id) != erased.end());

	objs.insert({id, std::move(erased[id])});
	erased.erase(id);
}

void Scene::erase(Scene_Object::ID id) {
	assert(erased.find(id) == erased.end());
	assert(objs.find(id) != objs.end());

	erased.insert({id, std::move(objs[id])});
	objs.erase(id);
}

void Scene::render_objs(Mat4 view, const GL::Shader& shader, Scene_Object::ID selected) {
	for(auto& obj : objs) {
		if(obj.first != selected) 
			obj.second.render(view, shader);
	}
}

void Scene::for_objs(std::function<void(Scene_Object&)> func) {
	for(auto& obj : objs) {
		func(obj.second);
	}
}

size_t Scene::size() {
	return objs.size();
}

bool Scene::empty() {
	return objs.size() == 0;
}

std::optional<std::reference_wrapper<Scene_Object>> Scene::get(Scene_Object::ID id) {
	auto entry = objs.find(id);
	if(entry == objs.end()) return std::nullopt;
	return entry->second;
}

void Scene::clear(Undo& undo) {
	next_id = first_id;
	objs.clear();
	erased.clear();
	undo.reset();
}

std::string Scene::load_scene(bool clear_first, Undo& undo, std::string file) {

	if(clear_first) clear(undo);
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file.c_str(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);

	if (!scene) {
		return "Error parsing scene " + file + " : " + std::string(importer.GetErrorString());
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[i];
    	
		if(!mesh->HasNormals()) {
			warn("Mesh %d has no normals, skipping.");
			continue;
		}

		std::vector<GL::Mesh::Vert> verts;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			const aiVector3D* pos = &(mesh->mVertices[i]);
			const aiVector3D* norm = &(mesh->mNormals[i]);

			verts.push_back({Vec3(pos->x, pos->y, pos->z), Vec3(norm->x, norm->y, norm->z)});
		}

		std::vector<GL::Mesh::Vert> expand_verts;
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			const aiFace& face = mesh->mFaces[i];
			assert(face.mNumIndices == 3);
			expand_verts.push_back(verts[face.mIndices[0]]);
			expand_verts.push_back(verts[face.mIndices[1]]);
			expand_verts.push_back(verts[face.mIndices[2]]);
		}

		add({}, GL::Mesh(expand_verts));
    }
	return {};
}
