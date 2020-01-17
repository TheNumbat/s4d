
#include "render.h"
#include "../gui.h"

#include <imgui/imgui.h>

void Renderer::setup(Vec2 dim) {
	window_dim = dim;
	id_buffer = new GLubyte[(int)dim.x * (int)dim.y * 4];
	framebuffer.setup(2, dim, samples, true);
	id_resolve.setup(1, dim, 1, false);
    mesh_shader.load(GL::Shaders::mesh_v, GL::Shaders::mesh_f);
	line_shader.load(GL::Shaders::line_v, GL::Shaders::line_f);
}

void Renderer::update_dim(Vec2 dim) {
	window_dim = dim;
	delete[] id_buffer;
	id_buffer = new GLubyte[(int)dim.x * (int)dim.y * 4]();
	framebuffer.resize(dim, samples);
	id_resolve.resize(dim);
}

void Renderer::shutdown() {
	mesh_shader.~Shader();
	line_shader.~Shader();
	framebuffer.~Framebuffer();
	id_resolve.~Framebuffer();
	delete[] id_buffer;
	id_buffer = nullptr;
}

void Renderer::proj(Mat4 proj) {
	_proj = proj;
}

void Renderer::complete() {

	framebuffer.blit_to(1, id_resolve, false);
	
	if(!id_resolve.can_read_at())
		id_resolve.read(0, id_buffer);

	framebuffer.blit_to_screen(0, window_dim);
}

void Renderer::begin() {
	framebuffer.clear(0, Vec4(Gui::Color::background, 1.0f));
	framebuffer.clear(1, {0.0f, 0.0f, 0.0f, 1.0f});
	framebuffer.clear_d();
	framebuffer.bind();
}

void Renderer::lines(const GL::Lines& lines, Mat4 viewproj, float alpha) {

	line_shader.bind();
	line_shader.uniform("viewproj", viewproj);
	line_shader.uniform("alpha", alpha);
	lines.render(framebuffer.is_multisampled());
}

void Renderer::mesh(const GL::Mesh& mesh, MeshOpt opt) {

    mesh_shader.bind();
	mesh_shader.uniform("proj", _proj);
	mesh_shader.uniform("use_v_id", opt.per_vert_id);
	mesh_shader.uniform("id", opt.id);
	mesh_shader.uniform("modelview", opt.modelview);
	mesh_shader.uniform("normal", opt.normal);
	mesh_shader.uniform("solid", opt.solid_color);
	
	if(opt.depth_only) GL::color_mask(false);

	if(opt.wireframe) {
		mesh_shader.uniform("color", Vec3());
		GL::enable(GL::Opt::wireframe);
		mesh.render();
		GL::disable(GL::Opt::wireframe);
	}

	mesh_shader.uniform("color", opt.color);
	mesh.render();

	if(opt.depth_only) GL::color_mask(true);
}

void Renderer::settings_gui(bool* open) {
	ImGui::Begin("Display Settings", open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
	
	ImGui::InputInt("Multisampling", &samples);
	if(samples < 1) samples = 1;
	if(samples > 16) samples = 16;

	if(ImGui::Button("Apply")) {
		framebuffer.resize(window_dim, samples);
	}

	ImGui::Separator();
	ImGui::Text("GPU: %s", GL::renderer().c_str());
	ImGui::Text("OpenGL: %s", GL::version().c_str());

	ImGui::End();
}

Scene_Object::ID Renderer::read_id(Vec2 pos) {
	
	int x = (int)pos.x;
	int y = (int)(window_dim.y - pos.y - 1);

	if(id_resolve.can_read_at()) {
		
		GLubyte data[4] = {};
		id_resolve.read_at(0, x, y, data);
		return (int)data[0] | (int)data[1] << 8 | (int)data[2] << 16;

	} else {
		int idx = y * (int)window_dim.x * 4 + x * 4;
	
		int a = id_buffer[idx];
		int b = id_buffer[idx + 1];
		int c = id_buffer[idx + 2];

		return a | b << 8 | c << 16;
	}
	return 0;
}

void Renderer::reset_depth() {
	framebuffer.clear_d();
}

void Renderer::outline(Mat4 viewproj, Mat4 view, const Scene_Object& obj) {

	framebuffer.clear_d();
	obj.render_mesh(view, false, true);

	Vec2 min, max;
	obj.bbox().screen_rect(viewproj, min, max);

	GL::flush_if_nvidia();
	GL::Effects::outline(framebuffer, framebuffer, Gui::Color::outline, 
						min - Vec2(3.0f / window_dim.x), 
						max + Vec2(3.0f / window_dim.y));
	GL::flush_if_nvidia();
}
