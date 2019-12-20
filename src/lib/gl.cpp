	
#include "gl.h"
#include "log.h"

#include <fstream>

namespace GL {

void global_params() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonOffset(1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
	glClearDepth(0.0);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
}

void clear_screen(Vec4 col) {
	Framebuffer::bind_screen();
	glClearColor(col.x, col.y, col.z, col.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void begin_wireframe() {
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void end_wireframe() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);
}

void begin_offset() {
	glEnable(GL_POLYGON_OFFSET_FILL);
}

void end_offset() {
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void start_stencil_only() {
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xff); 
	glStencilMask(0xff);
	glColorMask(false, false, false, false);
	glDepthMask(false);
}

void use_stencil() {
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glColorMask(true, true, true, true);
	glDepthMask(true);
}

void end_stencil_only() {
	glStencilMask(0xff);
	glDisable(GL_STENCIL_TEST);
	glColorMask(true, true, true, true);
	glDepthMask(true);
}

void viewport(Vec2 dim) {
	glViewport(0, 0, (GLsizei)dim.x, (GLsizei)dim.y);
}

Mesh::Mesh() {
	create();
}

Mesh::Mesh(const std::vector<Vert>& vertices) {
	create();
	update(vertices);
}

Mesh::Mesh(Mesh&& src) {
	vao = src.vao; src.vao = 0;
	vbo = src.vbo; src.vbo = 0;
	n_elem = src.n_elem; src.n_elem = 0;
}

void Mesh::operator=(Mesh&& src) {
	destroy();
	vao = src.vao; src.vao = 0;
	vbo = src.vbo; src.vbo = 0;
	n_elem = src.n_elem; src.n_elem = 0;
}

Mesh::~Mesh() {
	destroy();
}

void Mesh::create() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (GLvoid*)sizeof(Vec3));
	glEnableVertexAttribArray(1);
	
	glBindVertexArray(0);
}

void Mesh::destroy() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	vao = vbo = 0;
}

void Mesh::update(const std::vector<Vert>& vertices) {

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vert) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	n_elem = vertices.size();
}

void Mesh::render() const {

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, n_elem);
	glBindVertexArray(0);
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

void Lines::update() {

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Line_Vert) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);

	dirty = false;
}

void Lines::render() {

	if(dirty) update();

	glLineWidth(thickness);
	glEnable(GL_LINE_SMOOTH);

	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, vertices.size());
	glBindVertexArray(0);
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
	v_file = std::move(src.v_file);
	f_file = std::move(src.f_file);
	program = src.program; src.program = 0;
	v = src.v; src.v = 0;
	f = src.f; src.f = 0;
}

void Shader::operator=(Shader&& src) {
	destroy();
	v_file = std::move(src.v_file);
	f_file = std::move(src.f_file);
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

void Shader::reload() {
	destroy();
	load(v_file, f_file);
}

void Shader::destroy() {
	glUseProgram(0);
	glDeleteShader(v);
	glDeleteShader(f);
	glDeleteProgram(program);
	v = f = program = 0;
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

void Shader::uniform(std::string name, bool b) const {
	glUniform1i(loc(name), b);
}

GLuint Shader::loc(std::string name) const {

	return glGetUniformLocation(program, name.c_str());
}

void Shader::load(std::string vertex, std::string fragment) {

	v_file = vertex;
	f_file = fragment;
	std::string vs, fs;
	std::ifstream vfin(vertex), ffin(fragment);
	std::getline(vfin, vs, '\0');
	std::getline(ffin, fs, '\0');

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* vs_c = vs.c_str();
	const GLchar* fs_c = fs.c_str();
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

Framebuffer::Framebuffer(int outputs, Vec2 dim, int samples, bool d) {
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
			glTexImage2DMultisample(type, s, GL_DEPTH32F_STENCIL8, w, h, GL_TRUE);
		} else {
			glTexImage2D(type, 0, GL_DEPTH32F_STENCIL8, w, h, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_BYTE, 0);
			glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, type, depth_tex, 0);
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

void Framebuffer::clear_ds() const {
	bind();
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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

void Framebuffer::read(int buf, float* data) const {
	assert(s == 1);
	assert(buf >= 0 && buf < (int)output_textures.size());
	glBindTexture(GL_TEXTURE_2D, output_textures[buf]);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, data);
}

void Framebuffer::blit_to(int buf, const Framebuffer& fb, bool avg) const {

	assert(buf >= 0 && buf < (int)output_textures.size());
	if(s > 1) {
		Resolve::to(buf, *this, fb, avg);
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
		Resolve::to_screen(buf, *this);
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

void Resolve::init() {

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	resolve_shader.load("resolve.vert", "resolve.frag");
}

void Resolve::destroy() {

	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	vao = vbo = 0;
	resolve_shader.~Shader();
}

void Resolve::to_screen(int buf, const Framebuffer& framebuffer) {

	Framebuffer::bind_screen();

	resolve_shader.bind();
	glBindVertexArray(vao);
	
	assert(buf >= 0 && buf < (int)framebuffer.output_textures.size());
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebuffer.output_textures[buf]);

	resolve_shader.uniform("tex", 0);
	resolve_shader.uniform("samples", framebuffer.s);
	resolve_shader.uniform("tex_size", Vec2(framebuffer.w, framebuffer.h));

	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

void Resolve::to(int buf, const Framebuffer& from, const Framebuffer& to, bool avg) {

	to.bind();

	resolve_shader.bind();
	glBindVertexArray(vao);
	
	assert(buf >= 0 && buf < (int)from.output_textures.size());
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, from.output_textures[buf]);

	resolve_shader.uniform("tex", 0);
	resolve_shader.uniform("samples", avg ? from.s : 1);
	resolve_shader.uniform("tex_size", Vec2(from.w, from.h));

	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);
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

void setup() {
	setup_debug_proc();
	Resolve::init();
}

void shutdown() {
	Resolve::destroy();
	check_leaked_handles();
}
}
