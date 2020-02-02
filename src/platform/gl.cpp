	
#include "gl.h"
#include "../lib/log.h"

#include <fstream>

namespace GL {

static void setup_debug_proc();
static void check_leaked_handles();
static bool is_nvidia = false;
static bool is_gl45 = false;

void setup() {
	std::string ver = version();
	is_nvidia = ver.find("NVIDIA") != std::string::npos;
	is_gl45 = ver.find("4.5") != std::string::npos;

	setup_debug_proc();
	Effects::init();
}

void shutdown() {
	Effects::destroy();
	check_leaked_handles();
}

void flush_if_nvidia() {
	if(is_nvidia) glFlush();
}

void color_mask(bool enable) {
	glColorMask(enable, enable, enable, enable);
}

std::string version() {
	return std::string((char*)glGetString(GL_VERSION));
}
std::string renderer() {
	return std::string((char*)glGetString(GL_RENDERER));
}

void global_params() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonOffset(1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
	glClearDepth(0.0);
	if(glClipControl) glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glCullFace(GL_BACK);
}

void clear_screen(Vec4 col) {
	Framebuffer::bind_screen();
	glClearColor(col.x, col.y, col.z, col.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void enable(Opt opt) {
	switch(opt) {
	case Opt::wireframe: {
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} break;
	case Opt::offset: {
		glEnable(GL_POLYGON_OFFSET_FILL);
	} break;
	case Opt::culling: {
		glEnable(GL_CULL_FACE);
	} break;
	}
}

void disable(Opt opt) {
	switch(opt) {
	case Opt::wireframe: {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
	} break;
	case Opt::offset: {
		glDisable(GL_POLYGON_OFFSET_FILL);
	} break;
	case Opt::culling: {
		glDisable(GL_CULL_FACE);
	} break;
	}
}

void viewport(Vec2 dim) {
	glViewport(0, 0, (GLsizei)dim.x, (GLsizei)dim.y);
}

Mesh::Mesh() {
	create();
}

Mesh::Mesh(std::vector<Vert>&& vertices, std::vector<Index>&& indices) {
	create();
	update(std::move(vertices), std::move(indices));
}

Mesh::Mesh(Mesh&& src) {
	vao = src.vao; src.vao = 0;
	ebo = src.ebo; src.ebo = 0;
	vbo = src.vbo; src.vbo = 0;
	n_elem = src.n_elem; src.n_elem = 0;
	_bbox = src._bbox; src._bbox.reset();
	_verts = std::move(src._verts);
	_idxs = std::move(src._idxs);
}

void Mesh::operator=(Mesh&& src) {
	destroy();
	vao = src.vao; src.vao = 0;
	vbo = src.vbo; src.vbo = 0;
	ebo = src.ebo; src.ebo = 0;
	n_elem = src.n_elem; src.n_elem = 0;
	_bbox = src._bbox; src._bbox.reset();
	_verts = std::move(src._verts);
	_idxs = std::move(src._idxs);
}

Mesh::~Mesh() {
	destroy();
}

void Mesh::create() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (GLvoid*)sizeof(Vec3));
	glEnableVertexAttribArray(1);

	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vert), (GLvoid*)(2 * sizeof(Vec3)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	
	glBindVertexArray(0);
}

void Mesh::destroy() {
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	ebo = vao = vbo = 0;
}

void Mesh::update(std::vector<Vert>&& vertices, std::vector<Index>&& indices) {

	_verts = std::move(vertices);
	_idxs = std::move(indices);
	
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vert) * _verts.size(), _verts.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * _idxs.size(), _idxs.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	_bbox.reset();
	for(auto& v : _verts) {
		_bbox.enclose(v.pos);
	}
	n_elem = _idxs.size();
}

GLuint Mesh::tris() const {
	return n_elem / 3;
}

const std::vector<Mesh::Vert>& Mesh::verts() const {
	return _verts;
}

const std::vector<Mesh::Index>& Mesh::indices() const {
	return _idxs;
}

BBox Mesh::bbox() const {
	return _bbox;
}

void Mesh::render() const {
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, n_elem, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

Instances::Instances(GL::Mesh&& mesh) : mesh(std::move(mesh)) {
	create();
}

Instances::Instances(Instances&& src) {
	mesh = std::move(src.mesh);
	data = std::move(src.data);
	vbo = src.vbo; src.vbo = 0;
	dirty = src.dirty; src.dirty = false;
}

Instances::~Instances() {
	destroy();
}

void Instances::operator=(Instances&& src) {
	mesh = std::move(src.mesh);
	data = std::move(src.data);
	vbo = src.vbo; src.vbo = 0;
	dirty = src.dirty; src.dirty = false;
}

void Instances::create() {
	glGenBuffers(1, &vbo);
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Info), (GLvoid*)0);
	glVertexAttribDivisor(3, 1);

	const int base_idx = 4;
	for(int i = 0; i < 4; i++) {
		glEnableVertexAttribArray(base_idx + i);
		glVertexAttribPointer(base_idx + i, 4, GL_FLOAT, GL_FALSE, sizeof(Info), (void*)(sizeof(GLuint) + sizeof(Vec4) * i));
		glVertexAttribDivisor(base_idx + i, 1);
	}
	glBindVertexArray(0);
}

void Instances::render() {
	
	if(dirty) update();
	glBindVertexArray(mesh.vao);
	glDrawElementsInstanced(GL_TRIANGLES, mesh.n_elem, GL_UNSIGNED_INT, nullptr, data.size());
	glBindVertexArray(0);
}

void Instances::add(Mat4 transform, GLuint id) {
	data.push_back({id, transform});
	dirty = true;
}

void Instances::clear() {
	data.clear();
	dirty = true;
}

void Instances::update() {

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Info) * data.size(), data.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);

	dirty = false;
}

void Instances::destroy() {
	glDeleteBuffers(1, &vbo);
	vbo = 0;
	mesh.destroy();
}

Lines::Lines(float thickness) : thickness(thickness) {
	create();
}

Lines::Lines(Lines&& src) {
	dirty = src.dirty; src.dirty = false;
	thickness = src.thickness; src.thickness = 0.0f;
	vao = src.vao; src.vao = 0;
	vbo = src.vbo; src.vbo = 0;
	vertices = std::move(src.vertices);
}

void Lines::operator=(Lines&& src) {
	destroy();
	dirty = src.dirty; src.dirty = false;
	thickness = src.thickness; src.thickness = 0.0f;
	vao = src.vao; src.vao = 0;
	vbo = src.vbo; src.vbo = 0;
	vertices = std::move(src.vertices);
}

Lines::~Lines() {
	destroy();
}

void Lines::update() const {

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Line_Vert) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);

	dirty = false;
}

void Lines::render(bool smooth) const {

	if(dirty) update();

	glLineWidth(thickness);
	if(smooth) glEnable(GL_LINE_SMOOTH);
	else glDisable(GL_LINE_SMOOTH);

	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, vertices.size());
	glBindVertexArray(0);
}

void Lines::clear() {
	vertices.clear();
	dirty = true;
}

void Lines::pop() {
	vertices.pop_back();
	vertices.pop_back();
	dirty = true;
}

void Lines::add(Vec3 start, Vec3 end, Vec3 color) {

	vertices.push_back({start, color});
	vertices.push_back({end, color});
	dirty = true;
}

void Lines::create() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Line_Vert), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Line_Vert), (GLvoid*)sizeof(Vec3));
	glEnableVertexAttribArray(1);
	
	glBindVertexArray(0);
}

void Lines::destroy() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	vao = vbo = 0;
	vertices.clear();
	dirty = false;
}

Shader::Shader() {}

Shader::Shader(std::string vertex, std::string fragment) {
	load(vertex, fragment);
}

Shader::Shader(Shader&& src) {
	program = src.program; src.program = 0;
	v = src.v; src.v = 0;
	f = src.f; src.f = 0;
}

void Shader::operator=(Shader&& src) {
	destroy();
	program = src.program; src.program = 0;
	v = src.v; src.v = 0;
	f = src.f; src.f = 0;
}

Shader::~Shader() {
	destroy();
}

void Shader::bind() const {
	glUseProgram(program);
}

void Shader::destroy() {
	glUseProgram(0);
	glDeleteShader(v);
	glDeleteShader(f);
	glDeleteProgram(program);
	v = f = program = 0;
}

void Shader::uniform(std::string name, int count, const Vec2 items[]) const {
	glUniform2fv(loc(name), count, (GLfloat*)items);
}

void Shader::uniform(std::string name, GLfloat f) const {
	glUniform1f(loc(name), f);
}

void Shader::uniform(std::string name, Mat4 mat) const {
	glUniformMatrix4fv(loc(name), 1, GL_FALSE, mat.data);
}

void Shader::uniform(std::string name, Vec3 vec3) const {
	glUniform3fv(loc(name), 1, vec3.data);
}

void Shader::uniform(std::string name, Vec2 vec2) const {
	glUniform2fv(loc(name), 1, vec2.data);
}

void Shader::uniform(std::string name, GLint i) const {
	glUniform1i(loc(name), i);
}

void Shader::uniform(std::string name, GLuint i) const {
	glUniform1ui(loc(name), i);
}

void Shader::uniform(std::string name, bool b) const {
	glUniform1i(loc(name), b);
}

GLuint Shader::loc(std::string name) const {

	return glGetUniformLocation(program, name.c_str());
}

void Shader::load(std::string vertex, std::string fragment) {

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* vs_c = vertex.c_str();
	const GLchar* fs_c = fragment.c_str();
	glShaderSource(v, 1, &vs_c, NULL);
	glShaderSource(f, 1, &fs_c, NULL);
	glCompileShader(v);
	glCompileShader(f);

	if(!validate(v)) {
		destroy();
		return;
	}
	if(!validate(f)) {
		destroy();
		return;
	}

	program = glCreateProgram();
	glAttachShader(program, v);
	glAttachShader(program, f);
	glLinkProgram(program);
}

bool Shader::validate(GLuint program) {

	GLint compiled = 0;
	glGetShaderiv(program, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE) {
		
		GLint len = 0;
		glGetShaderiv(program, GL_INFO_LOG_LENGTH, &len);

		GLchar* msg = new GLchar[len];
		glGetShaderInfoLog(program, len, &len, msg);

		warn("Shader %d failed to compile: %s", program, msg);
		delete[] msg;

		return false;
	}
	return true;
}

Framebuffer::Framebuffer() {}

Framebuffer::Framebuffer(int outputs, Vec2 dim, int samples, bool d) {
	setup(outputs, dim, samples, d);
}

void Framebuffer::setup(int outputs, Vec2 dim, int samples, bool d) {
	destroy();
	assert(outputs >= 0 && outputs < 31);
	depth = d;
	output_textures.resize(outputs);
	resize(dim, samples);
}

Framebuffer::Framebuffer(Framebuffer&& src) {
	output_textures = std::move(src.output_textures);
	depth_tex = src.depth_tex; src.depth_tex = 0;
	framebuffer = src.framebuffer; src.framebuffer = 0;
	w = src.w; src.w = 0;
	h = src.h; src.h = 0;
	s = src.s; src.s = 0;
}

void Framebuffer::operator=(Framebuffer&& src) {
	destroy();
	output_textures = std::move(src.output_textures);
	depth_tex = src.depth_tex; src.depth_tex = 0;
	framebuffer = src.framebuffer; src.framebuffer = 0;
	w = src.w; src.w = 0;
	h = src.h; src.h = 0;
	s = src.s; src.s = 0;
}

Framebuffer::~Framebuffer() {
	destroy();
}

void Framebuffer::create() {
	glGenFramebuffers(1, &framebuffer);
	glGenTextures(output_textures.size(), output_textures.data());
	if(depth) glGenTextures(1, &depth_tex);
}

void Framebuffer::destroy() {
	glDeleteTextures(1, &depth_tex);
	glDeleteTextures(output_textures.size(), output_textures.data());
	glDeleteFramebuffers(1, &framebuffer);
	depth_tex = framebuffer = 0;
}

void Framebuffer::resize(Vec2 dim, int samples) {

	destroy();
	create();
	
	w = (int)dim.x;
	h = (int)dim.y;
	s = samples;
	assert(w > 0 && h > 0 && s > 0);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	GLenum type = samples == 1 ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE;

	std::vector<GLenum> draw_buffers;

	for(size_t i = 0; i < output_textures.size(); i++) {

		glBindTexture(type, output_textures[i]);

		if(s > 1) {
			glTexImage2DMultisample(type, s, GL_RGB8, w, h, GL_TRUE);
		} else {
			glTexImage2D(type, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
			glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, type, output_textures[i], 0);

		draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + i);

		glBindTexture(type, 0);
	}

	if(depth) {
		glBindTexture(type, depth_tex);
		if(s > 1) {
			glTexImage2DMultisample(type, s, GL_DEPTH_COMPONENT32F, w, h, GL_TRUE);
		} else {
			glTexImage2D(type, 0, GL_DEPTH_COMPONENT32F, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, type, depth_tex, 0);
		glBindTexture(type, 0);
	}

	glDrawBuffers(draw_buffers.size(), draw_buffers.data());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::clear(int buf, Vec4 col) const {
	assert(buf >= 0 && buf < (int)output_textures.size());
	bind();
	glClearBufferfv(GL_COLOR, buf, col.data);
}

void Framebuffer::clear_d() const {
	bind();
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::bind_screen() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

GLuint Framebuffer::get_output(int buf) const {
	assert(buf >= 0 && buf < (int)output_textures.size());
	return output_textures[buf];
}

GLuint Framebuffer::get_depth() const {
	assert(depth_tex);
	return depth_tex;
}

bool Framebuffer::can_read_at() const {
	return is_gl45 && s == 1;
}

void Framebuffer::read_at(int buf, int x, int y, GLubyte* data) const {
	assert(can_read_at());
	assert(buf >= 0 && buf < (int)output_textures.size());
	glGetTextureSubImage(output_textures[buf], 0, x, y, 0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, 4, data);
}

void Framebuffer::read(int buf, GLubyte* data) const {
	assert(s == 1);
	assert(buf >= 0 && buf < (int)output_textures.size());
	glBindTexture(GL_TEXTURE_2D, output_textures[buf]);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void Framebuffer::blit_to(int buf, const Framebuffer& fb, bool avg) const {

	assert(buf >= 0 && buf < (int)output_textures.size());
	if(s > 1) {
		Effects::resolve_to(buf, *this, fb, avg);
		return;
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.framebuffer);

	glReadBuffer(GL_COLOR_ATTACHMENT0 + buf);
	glBlitFramebuffer(0, 0, w, h, 0, 0, fb.w, fb.h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::blit_to_screen(int buf, Vec2 dim) const {

	assert(buf >= 0 && buf < (int)output_textures.size());
	if(s > 1) {
		Effects::resolve_to_screen(buf, *this);
		return;
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glReadBuffer(GL_COLOR_ATTACHMENT0 + buf);
	glBlitFramebuffer(0, 0, w, h, 0, 0, (GLint)dim.x, (GLint)dim.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::is_multisampled() const {
	return s > 1;
}

void Effects::init() {

	glGenVertexArrays(1, &vao);
	resolve_shader.load(effects_v, resolve_f);
	outline_shader.load(effects_v, outline_f);
	outline_shader_ms.load(effects_v, is_gl45 ? outline_ms_f_4 : outline_ms_f_33);
}

void Effects::destroy() {

	glDeleteVertexArrays(1, &vao);
	vao = 0;
	resolve_shader.~Shader();
	outline_shader.~Shader();
	outline_shader_ms.~Shader();
}

void Effects::outline(const Framebuffer& from, const Framebuffer& to, Vec3 color, Vec2 min, Vec2 max) {

	to.bind();

	Vec2 quad[] = {
		{min.x, max.y},
		min,
		max,
		{max.x, min.y}
	};

	if(from.is_multisampled()) {	
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, from.get_depth());
		outline_shader_ms.bind();
		outline_shader_ms.uniform("depth", 0);
		outline_shader_ms.uniform("color", color);
		outline_shader_ms.uniform("bounds", 4, quad);
	} else {
		glBindTexture(GL_TEXTURE_2D, from.get_depth());
		outline_shader.bind();
		outline_shader.uniform("depth", 0);
		outline_shader.uniform("color", color);
		outline_shader.uniform("i_screen_size", 1.0f / Vec2(from.w, from.h));
		outline_shader.uniform("bounds", 4, quad);
	}

	glBindVertexArray(vao);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

void Effects::resolve_to_screen(int buf, const Framebuffer& framebuffer) {

	Framebuffer::bind_screen();

	resolve_shader.bind();
	
	assert(buf >= 0 && buf < (int)framebuffer.output_textures.size());
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebuffer.output_textures[buf]);

	resolve_shader.uniform("tex", 0);
	resolve_shader.uniform("samples", framebuffer.s);
	resolve_shader.uniform("bounds", 4, screen_quad);

	glBindVertexArray(vao);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

void Effects::resolve_to(int buf, const Framebuffer& from, const Framebuffer& to, bool avg) {

	to.bind();

	resolve_shader.bind();
	
	assert(buf >= 0 && buf < (int)from.output_textures.size());
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, from.output_textures[buf]);

	resolve_shader.uniform("tex", 0);
	resolve_shader.uniform("samples", avg ? from.s : 1);
	resolve_shader.uniform("bounds", 4, screen_quad);

	glBindVertexArray(vao);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

static void debug_proc(GLenum glsource, GLenum gltype, GLuint id, GLenum severity, GLsizei length, const GLchar* glmessage, const void* up) {

	std::string message(glmessage);
	std::string source, type;

	switch(glsource) {
	case GL_DEBUG_SOURCE_API:
		source = "OpenGL API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		source = "Window System";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		source = "Shader Compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		source = "Third Party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		source = "Application";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		source = "Other";
		break;
	}

	switch(gltype) {
	case GL_DEBUG_TYPE_ERROR:
		type = "Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		type = "Deprecated";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		type = "Undefined Behavior";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		type = "Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		type = "Performance";
		break;
	case GL_DEBUG_TYPE_MARKER:
		type = "Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		type = "Push Group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		type = "Pop Group";
		break;
	case GL_DEBUG_TYPE_OTHER:
		type = "Other";
		break;
	}

	switch(severity) {
	case GL_DEBUG_SEVERITY_HIGH:
	case GL_DEBUG_SEVERITY_MEDIUM:
		warn("OpenGL | source: %s type: %s message: %s", source.c_str(), type.c_str(), message.c_str());
		break;
	}
}

static void check_leaked_handles() {

	#define GL_CHECK(type) if(glIs##type && glIs##type(i) == GL_TRUE) { warn("Leaked OpenGL handle %u of type %s", i, #type); leaked = true;}

	bool leaked = false;
	for(GLuint i = 0; i < 10000; i++) {
		GL_CHECK(Texture);
		GL_CHECK(Buffer);
		GL_CHECK(Framebuffer);
		GL_CHECK(Renderbuffer);
		GL_CHECK(VertexArray);
		GL_CHECK(Program);
		GL_CHECK(ProgramPipeline);
		GL_CHECK(Query);

		if(glIsShader(i) == GL_TRUE) {

			leaked = true;
			GLint shader_len = 0;
			glGetShaderiv(i, GL_SHADER_SOURCE_LENGTH, &shader_len);

			GLchar* shader = new GLchar[shader_len];
			glGetShaderSource(i, shader_len, nullptr, shader);

			warn("Leaked OpenGL shader %u. Source: %s", i, shader); 

			delete[] shader;
		}
	}

	if(leaked) {
		warn("Leaked OpenGL objects!");
	}

	#undef GL_CHECK
}

static void setup_debug_proc() {
	if(!glDebugMessageCallback || !glDebugMessageControl) return;
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(debug_proc, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}

const std::string Effects::effects_v = R"(
#version 330 core

layout (location = 0) in vec2 v_pos;

uniform vec2 bounds[4];

void main() {
	gl_Position = vec4(bounds[gl_VertexID], 0.0f, 1.0f);
})";
const std::string Effects::outline_f = R"(
#version 330 core

uniform sampler2D depth;
uniform vec3 color;
uniform vec2 i_screen_size;
out vec4 out_color;

void main() {

	ivec2 coord = ivec2(gl_FragCoord.xy);
	float o = texture(depth, coord * i_screen_size).r;

	float low = 1.0f / 0.0f;
	for (int i = -2; i <= 2; i++) {
		for (int j = -2; j <= 2; j++) {
			float d = texture(depth, (coord + ivec2(i,j)) * i_screen_size).r;
			low = min(low, d);
		}
	}

	float a = o != 0.0f && low == 0.0f ? 1.0f : 0.0f;
	out_color = vec4(color * a, a);
})";
const std::string Effects::outline_ms_f_4 = R"(
#version 400 core

uniform sampler2DMS depth;
uniform vec3 color;
out vec4 out_color;

void main() {

	ivec2 coord = ivec2(gl_FragCoord.xy);
	float o = texelFetch(depth, coord, gl_SampleID).r;

	float low = 1.0f / 0.0f;
	for (int i = -2; i <= 2; i++) {
		for (int j = -2; j <= 2; j++) {
			float d = texelFetch(depth, coord + ivec2(i,j), gl_SampleID).r;
			low = min(low, d);
		}
	}

	float a = o != 0.0f && low == 0.0f ? 1.0f : 0.0f;
	out_color = vec4(color * a, a);
})";
const std::string Effects::outline_ms_f_33 = R"(
#version 330 core

uniform sampler2DMS depth;
uniform vec3 color;
out vec4 out_color;

void main() {

	ivec2 coord = ivec2(gl_FragCoord.xy);
	float o = texelFetch(depth, coord, 0).r;

	float low = 1.0f / 0.0f;
	for (int i = -2; i <= 2; i++) {
		for (int j = -2; j <= 2; j++) {
			float d = texelFetch(depth, coord + ivec2(i,j), 0).r;
			low = min(low, d);
		}
	}

	float a = o != 0.0f && low == 0.0f ? 1.0f : 0.0f;
	out_color = vec4(color * a, a);
})";
const std::string Effects::resolve_f = R"(
#version 330 core

uniform sampler2DMS tex;
uniform int samples;
out vec4 out_color;

void main() {

	ivec2 coord = ivec2(gl_FragCoord.xy);

	vec3 color = vec3(0.0);

	for (int i = 0; i < samples; i++)
		color += texelFetch(tex, coord, i).xyz;

	color /= float(samples);

	out_color = vec4(color, 1.0f);
})";

namespace Shaders {
	const std::string line_v = R"(
#version 330 core

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_col;

uniform mat4 viewproj;
smooth out vec3 f_col;

void main() {
	gl_Position = viewproj * vec4(v_pos, 1.0f);
	f_col = v_col;
})";
	const std::string line_f = R"(
#version 330 core

layout (location = 0) out vec4 out_col;
layout (location = 1) out vec4 out_id;

uniform float alpha;
smooth in vec3 f_col;

void main() {
	out_id = vec4(0.0f);
	out_col = vec4(f_col, alpha);
})"; 
	const std::string mesh_v = R"(
#version 330 core

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_norm;
layout (location = 2) in uint v_id;

uniform mat4 mvp, normal;
uniform vec3 color;

smooth out vec3 f_norm, f_color;
flat out uint f_id;

void main() {
	f_id = v_id;
	f_color = color;
	f_norm = (normal * vec4(v_norm, 0.0f)).xyz;
	gl_Position = mvp * vec4(v_pos, 1.0f);
})";
	const std::string inst_v = R"(
#version 330 core

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_norm;
layout (location = 2) in uint v_id;

layout (location = 3) in uint i_id;
layout (location = 4) in mat4 i_trans;

uniform bool use_i_id;
uniform mat4 proj, modelview;

uniform uint sel_id;
uniform vec3 color, sel_color;

smooth out vec3 f_norm, f_color;
flat out uint f_id;

void main() {
	f_id = use_i_id ? i_id : v_id;
	f_color = sel_id == f_id ? sel_color : color;
	mat4 mv = modelview * i_trans;
	mat4 n = transpose(inverse(mv));
	f_norm = (n * vec4(v_norm, 0.0f)).xyz;
	gl_Position = proj * mv * vec4(v_pos, 1.0f);
})";
	const std::string mesh_f = R"(
#version 330 core

uniform bool solid, use_v_id;
uniform uint id;

layout (location = 0) out vec4 out_col;
layout (location = 1) out vec4 out_id;

smooth in vec3 f_color;
smooth in vec3 f_norm;
flat in uint f_id;

void main() {

	if(use_v_id) {
		out_id = vec4((f_id & 0xffu) / 255.0f, ((f_id >> 8) & 0xffu) / 255.0f, ((f_id >> 16) & 0xffu) / 255.0f, 1.0f);
	} else {
		out_id = vec4((id & 0xffu) / 255.0f, ((id >> 8) & 0xffu) / 255.0f, ((id >> 16) & 0xffu) / 255.0f, 1.0f);
	}

	if(solid) {
		out_col = vec4(f_color, 1.0f);
	} else {
		float ndotl = max(normalize(f_norm).z, 0.0f);
		float light = clamp(0.2f + ndotl, 0.0f, 1.0f);
		out_col = vec4(light * f_color, 1.0f);
	}
})"; 

}
}
