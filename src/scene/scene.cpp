
#include "scene.h"
#include "render.h"
#include "../lib/log.h"
#include "../undo.h"

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>

#include <sstream>

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
	_mesh(std::move(src._mesh)),
	halfedge(std::move(src.halfedge)) {

	opt.name = std::move(src.opt.name);
	opt.wireframe = src.opt.wireframe; src.opt.wireframe = false;
	_id = src._id; src._id = 0;
	color = src.color; src.color = {};
	pose = src.pose; src.pose = {};
	mesh_dirty = src.mesh_dirty; src.mesh_dirty = false;
}

Scene_Object::Scene_Object(ID id, Pose p, GL::Mesh&& m, Vec3 c) :
	pose(p),
	color(c),
	_id(id),
	_mesh(std::move(m)) {
	
	mesh_dirty = false;
	editable = false;
	opt.name.reserve(max_name_len);
	snprintf(opt.name.data(), opt.name.capacity(), "Object %d", id);
}

Scene_Object::Scene_Object(ID id, Pose p, Halfedge_Mesh&& m, Vec3 c) :
	pose(p),
	color(c),
	_id(id),
	halfedge(std::move(m)) {
	
	mesh_dirty = true;
	editable = true;
	opt.name.reserve(max_name_len);
	snprintf(opt.name.data(), opt.name.capacity(), "Object %d", id);
}

Scene_Object::~Scene_Object() {

}

void Scene_Object::operator=(Scene_Object&& src) {
	_mesh = std::move(src._mesh);
	halfedge = std::move(src.halfedge);
	opt.name = std::move(src.opt.name);
	opt.wireframe = src.opt.wireframe; src.opt.wireframe = false;
	_id = src._id; src._id = 0;
	color = src.color; src.color = {};
	pose = src.pose; src.pose = {};
	mesh_dirty = src.mesh_dirty; src.mesh_dirty = false;
}

void Scene_Object::sync_mesh() const {
	if(editable && mesh_dirty) {
		halfedge.to_mesh(_mesh, true);
		mesh_dirty = false;
	}
}

BBox Scene_Object::bbox() const {

	Mat4 t = pose.transform();
	BBox ret;
	std::vector<Vec3> c = _mesh.bbox().corners();
	for(auto& v : c) ret.enclose(t * v);
	return ret;
}

void Scene_Object::render_halfedge(Mat4 view) const {

	Renderer::HalfedgeOpt opt;
	opt.modelview = view * pose.transform();
	opt.color = color;
	Renderer::halfedge(_mesh, halfedge, opt);
}

void Scene_Object::render_mesh(Mat4 view, bool solid, bool depth_only) const {

	sync_mesh();
	
	Renderer::MeshOpt opt;
	opt.modelview = view * pose.transform();
	opt.id = _id;
	opt.solid_color = solid;
	opt.depth_only = depth_only;
	opt.color = color;
	Renderer::mesh(_mesh, opt);
}

Scene::Scene(Scene_Object::ID start) :
	next_id(start),
	first_id(start) {
}

Scene::~Scene() {

}

Scene_Object::ID Scene::reserve_id() {
	return next_id++;
}

Scene_Object::ID Scene::add(Pose pose, GL::Mesh&& mesh, Scene_Object::ID id) {
	if(!id) id = next_id++;
	assert(objs.find(id) == objs.end());
	objs.emplace(std::make_pair(id, Scene_Object(id, pose, std::move(mesh))));
	return id;
}

Scene_Object::ID Scene::add(Scene_Object&& obj) {
	assert(objs.find(obj.id()) == objs.end());
	objs.emplace(std::make_pair(obj.id(), std::move(obj)));
	return obj.id();
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

void Scene::render_objs(Mat4 view, Scene_Object::ID selected) {
	for(auto& obj : objs) {
		if(obj.first != selected) 
			obj.second.render_mesh(view);
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

void Scene::load_node(std::vector<std::string>& errors, const aiScene* scene, aiNode* node, aiMatrix4x4 transform) {

	transform = transform * node->mTransformation;

	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		
		if(!mesh->HasNormals()) {
			errors.push_back("Mesh has no normals.");
			continue;
		}

		std::vector<GL::Mesh::Vert> verts;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			const aiVector3D& pos = mesh->mVertices[i];
			const aiVector3D& norm = mesh->mNormals[i];
			verts.push_back({Vec3(pos.x, pos.y, pos.z), Vec3(norm.x, norm.y, norm.z)});
		}

		std::vector<std::vector<Halfedge_Mesh::Index>> polys;
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			const aiFace& face = mesh->mFaces[i];
			
			std::vector<Halfedge_Mesh::Index> poly;
			for(unsigned int j = 0; j < face.mNumIndices; j++) {
				poly.push_back(face.mIndices[j]);
			}
			polys.push_back(poly);
		}

		aiVector3D ascale, arot, apos;
		transform.Decompose(ascale, arot, apos);
		Vec3 pos(apos.x, apos.y, apos.z);
		Vec3 rot(arot.x, arot.y, arot.z);
		Vec3 scale(ascale.x, ascale.y, ascale.z);
		Pose p = {pos, Degrees(rot).range(0.0f, 360.0f), scale};

		Halfedge_Mesh hemesh;
		std::string err = hemesh.from_poly(polys, verts);
		if(!err.empty()) {
			errors.push_back(err);
		} else {
			Scene_Object obj(reserve_id(), p, std::move(hemesh));
			if(mesh->mName.length) {
				obj.opt.name = std::string(mesh->mName.C_Str());
			}
			add(std::move(obj));
		}
	}

	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		load_node(errors, scene, node->mChildren[i], transform);
	}
}

std::string Scene::load(bool clear_first, Undo& undo, std::string file) {

	if(clear_first) clear(undo);
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file.c_str(), aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

	if (!scene) {
		return "Parsing scene " + file + ": " + std::string(importer.GetErrorString());
	}

	std::vector<std::string> errors;
	load_node(errors, scene, scene->mRootNode, aiMatrix4x4());
	
	std::stringstream stream;
	for(int i = 0; i < errors.size(); i++) {
		stream << "Loading mesh " << i << ": " << errors[i] << std::endl;
	}
	return stream.str();
}

std::string Scene::write(std::string file) {
	
	if(objs.empty()) return {};

	aiScene scene;
	scene.mRootNode = new aiNode();

	// no materials
	scene.mMaterials = new aiMaterial*[1]();
	scene.mMaterials[0] = nullptr;
	scene.mNumMaterials = 1;
	scene.mMaterials[0] = new aiMaterial();

	// meshes
	size_t n_meshes = objs.size();
	scene.mMeshes = new aiMesh*[n_meshes]();
	scene.mNumMeshes = n_meshes;
	
	scene.mRootNode->mNumMeshes = 0;
	scene.mRootNode->mNumChildren = n_meshes;
	scene.mRootNode->mChildren = new aiNode*[n_meshes]();

	for(size_t i = 0; i < n_meshes; i++) {
		scene.mMeshes[i] = new aiMesh();
		scene.mRootNode->mChildren[i] = new aiNode();
		scene.mRootNode->mChildren[i]->mNumMeshes = 1;
		scene.mRootNode->mChildren[i]->mMeshes = new unsigned int(i);
	}

	size_t mesh_idx = 0;
	for(auto& entry : objs) {

		Scene_Object& obj = entry.second;
		aiMesh* ai_mesh = scene.mMeshes[mesh_idx];
		aiNode* ai_node = scene.mRootNode->mChildren[mesh_idx];

		const std::vector<GL::Mesh::Vert>& verts = obj.mesh().verts();

		ai_mesh->mVertices = new aiVector3D[verts.size()];
		ai_mesh->mNormals = new aiVector3D[verts.size()];
		ai_mesh->mNumVertices = verts.size();

		int j = 0;
		for(GL::Mesh::Vert v : verts) {
			ai_mesh->mVertices[j] = aiVector3D(v.pos.x, v.pos.y, v.pos.z);
			ai_mesh->mNormals[j] = aiVector3D(v.norm.x, v.norm.y, v.norm.z);
			j++;
		}

		ai_mesh->mFaces = new aiFace[verts.size() / 3];
		ai_mesh->mNumFaces = (unsigned int)(verts.size() / 3);

		int k = 0;
		for(size_t i = 0; i < (verts.size() / 3); i++) {
			aiFace &face = ai_mesh->mFaces[i];
			face.mIndices = new unsigned int[3];
			face.mNumIndices = 3;
			face.mIndices[0] = k;
			face.mIndices[1] = k+1;
			face.mIndices[2] = k+2;
			k = k + 3;
		}

		ai_mesh->mName = aiString(obj.opt.name);
		
		Mat4 trans = obj.pose.transform();
		ai_node->mTransformation = {trans[0][0], trans[1][0], trans[2][0], trans[3][0],
									trans[0][1], trans[1][1], trans[2][1], trans[3][1],
									trans[0][2], trans[1][2], trans[2][2], trans[3][2],
									trans[0][3], trans[1][3], trans[2][3], trans[3][3]};

		mesh_idx++;
	}

	Assimp::Exporter exporter;
	if(exporter.Export(&scene, "collada", file.c_str())) {
		return std::string(exporter.GetErrorString());
	}
	return {};
}
