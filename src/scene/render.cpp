
#include "render.h"
#include "util.h"
#include "../gui.h"

#include <imgui/imgui.h>

Renderer::Renderer(Vec2 dim) :
	samples(4),
	window_dim(dim),
	id_buffer(new GLubyte[(int)dim.x * (int)dim.y * 4]),
	framebuffer(2, dim, samples, true),
	id_resolve(1, dim, 1, false),
    mesh_shader(GL::Shaders::mesh_v, GL::Shaders::mesh_f),
	line_shader(GL::Shaders::line_v, GL::Shaders::line_f),
	inst_shader(GL::Shaders::inst_v, GL::Shaders::mesh_f),
	spheres(Util::sphere_mesh(0.1f, 1)),
	cylinders(Util::cyl_mesh(0.1f, 1.0f))
{}

Renderer::~Renderer() {
	delete[] id_buffer;
	id_buffer = nullptr;
}

void Renderer::setup(Vec2 dim) {
	data = new Renderer(dim);
}

void Renderer::update_dim(Vec2 dim) {
	assert(data);
	data->window_dim = dim;
	delete[] data->id_buffer;
	data->id_buffer = new GLubyte[(int)dim.x * (int)dim.y * 4]();
	data->framebuffer.resize(dim, data->samples);
	data->id_resolve.resize(dim);
}

void Renderer::shutdown() {
	delete data;
	data = nullptr;
}

void Renderer::proj(Mat4 proj) {
	assert(data);
	data->_proj = proj;
}

void Renderer::complete() {
	assert(data);
	data->framebuffer.blit_to(1, data->id_resolve, false);
	
	if(!data->id_resolve.can_read_at())
		data->id_resolve.read(0, data->id_buffer);

	data->framebuffer.blit_to_screen(0, data->window_dim);
}

void Renderer::begin() {
	assert(data);
	data->framebuffer.clear(0, Vec4(Gui::Color::background, 1.0f));
	data->framebuffer.clear(1, {0.0f, 0.0f, 0.0f, 1.0f});
	data->framebuffer.clear_d();
	data->framebuffer.bind();
}

void Renderer::lines(const GL::Lines& lines, Mat4 viewproj, float alpha) {
	assert(data);
	data->line_shader.bind();
	data->line_shader.uniform("viewproj", viewproj);
	data->line_shader.uniform("alpha", alpha);
	lines.render(data->framebuffer.is_multisampled());
}

void Renderer::mesh(const GL::Mesh& mesh, Renderer::MeshOpt opt) {
	assert(data);
    data->mesh_shader.bind();
	data->mesh_shader.uniform("use_v_id", opt.per_vert_id);
	data->mesh_shader.uniform("id", opt.id);
	data->mesh_shader.uniform("mvp", data->_proj * opt.modelview);
	data->mesh_shader.uniform("normal", Mat4::transpose(Mat4::inverse(opt.modelview)));
	data->mesh_shader.uniform("solid", opt.solid_color);
	
	if(opt.depth_only) GL::color_mask(false);

	if(opt.wireframe) {
		data->mesh_shader.uniform("color", Vec3());
		GL::enable(GL::Opt::wireframe);
		mesh.render();
		GL::disable(GL::Opt::wireframe);
	}

	data->mesh_shader.uniform("color", opt.color);
	mesh.render();

	if(opt.depth_only) GL::color_mask(true);
}

void Renderer::settings_gui(bool* open) {
	assert(data);
	ImGui::Begin("Display Settings", open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
	
	ImGui::InputInt("Multisampling", &data->samples);
	if(data->samples < 1) data->samples = 1;
	if(data->samples > 16) data->samples = 16;

	if(ImGui::Button("Apply")) {
		data->framebuffer.resize(data->window_dim, data->samples);
	}

	ImGui::Separator();
	ImGui::Text("GPU: %s", GL::renderer().c_str());
	ImGui::Text("OpenGL: %s", GL::version().c_str());

	ImGui::End();
}

Scene_Object::ID Renderer::read_id(Vec2 pos) {
	assert(data);
	int x = (int)pos.x;
	int y = (int)(data->window_dim.y - pos.y - 1);

	if(data->id_resolve.can_read_at()) {
		
		GLubyte read[4] = {};
		data->id_resolve.read_at(0, x, y, read);
		return (int)read[0] | (int)read[1] << 8 | (int)read[2] << 16;

	} else {
		int idx = y * (int)data->window_dim.x * 4 + x * 4;
	
		int a = data->id_buffer[idx];
		int b = data->id_buffer[idx + 1];
		int c = data->id_buffer[idx + 2];

		return a | b << 8 | c << 16;
	}
	return 0;
}

void Renderer::reset_depth() {
	assert(data);
	data->framebuffer.clear_d();
}

void Renderer::outline(Mat4 viewproj, Mat4 view, const Scene_Object& obj) {
	assert(data);
	data->framebuffer.clear_d();
	obj.render_mesh(view, false, true);

	Vec2 min, max;
	obj.bbox().screen_rect(viewproj, min, max);

	GL::flush_if_nvidia();
	GL::Effects::outline(data->framebuffer, data->framebuffer, Gui::Color::outline, 
						 min - Vec2(3.0f / data->window_dim.x), 
						 max + Vec2(3.0f / data->window_dim.y));
	GL::flush_if_nvidia();
}

void Renderer::build_halfedge(const Halfedge_Mesh& mesh) {

	if(!mesh.render_dirty_flag) return;
	mesh.render_dirty_flag = false;

	std::map<Halfedge_Mesh::VertexCRef, float> size;

	spheres.clear();
	for(auto v = mesh.vertices_begin(); v != mesh.vertices_end(); v++) {
		float d = FLT_MAX;
		auto he = v->halfedge();
		do {
			Vec3 n = he->twin()->vertex()->pos;
			float e = (n-v->pos).norm();
			d = d > e ? e : d;
			he = he->twin()->next();
		} while(he != v->halfedge());
		size[v] = d;
		spheres.add(Mat4::translate(v->pos) * Mat4::scale(d));
	}

	cylinders.clear();
	for(auto e = mesh.edges_begin(); e != mesh.edges_end(); e++) {
		auto v_0 = e->halfedge()->vertex();
		auto v_1 = e->halfedge()->twin()->vertex();
		Vec3 v0 = v_0->pos;
		Vec3 v1 = v_1->pos;
		
		Vec3 dir = v1 - v0;
		float l = dir.norm();
			  dir /= l;
		float s = 0.25f * (size[v_0] + size[v_1]);

		Mat4 rot;
		Vec3 x = cross(dir, {0.0f, 1.0f, 0.0f});
		Vec3 z = cross(x, dir);

		if(x.norm() != 0.0f) {
			rot = {{x, 0.0f}, {dir, 0.0f}, {z, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}};
		} else if(dir.y == -1.0f) {
			l = -l;
		}
		cylinders.add(Mat4::translate(v0) * rot * Mat4::scale({s, l, s}));
	}
}

void Renderer::halfedge(const GL::Mesh& faces, const Halfedge_Mesh& mesh, Renderer::HalfedgeOpt opt) {

	data->build_halfedge(mesh);

	assert(data);
	MeshOpt fopt;
	fopt.modelview = opt.modelview;
	fopt.color = opt.color;
	fopt.id = 0;
	Renderer::mesh(faces, fopt);

	data->inst_shader.bind();
	data->inst_shader.uniform("use_v_id", false);
	data->inst_shader.uniform("solid", false);
	data->inst_shader.uniform("id", 0);
	data->inst_shader.uniform("proj", data->_proj);
	data->inst_shader.uniform("modelview", opt.modelview);
	data->inst_shader.uniform("color", opt.color);

	data->spheres.render();
	data->cylinders.render();
}
