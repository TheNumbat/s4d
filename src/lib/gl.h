
#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

#include "math.h"

namespace GL {

void setup();
void shutdown();

void global_params();
void clear_screen(Vec4 col);
void viewport(Vec2 dim);

void begin_offset();
void end_offset();
void begin_wireframe();
void end_wireframe();
void start_stencil();
void use_stencil();
void end_stencil();

class Mesh {
public:
	struct Vert {
		Vec3 pos;
		Vec3 norm;
	};

	Mesh();
	Mesh(const std::vector<Vert>& vertices);
	Mesh(const Mesh& src) = delete;
	Mesh(Mesh&& src);
	~Mesh();

	/// Assumes proper shader is already bound
	void render();
	void update(const std::vector<Vert>& vertices);

private:
	void create();
	void destroy();

	GLuint vao = 0, vbo = 0;
	GLuint n_elem = 0;
};

class Lines {
public:
	Lines(float thickness);
	Lines(const Lines& src) = delete;
	Lines(Lines&& src);
	~Lines();

	/// Assumes proper shader is already bound
	void render();
	void add(Vec3 start, Vec3 end, Vec3 color);

private:
	void create();
	void destroy();
	void update();

	bool dirty = false;
	float thickness = 0.0f;
	GLuint vao = 0, vbo = 0;

	struct Line_Vert {
		Vec3 pos;
		Vec3 color;
	};

	std::vector<Line_Vert> vertices;
};

class Shader {	
public:
	Shader();
	Shader(std::string vertex_file, std::string fragment_file);
	Shader(const Shader& src) = delete;
	Shader(Shader&& src);
	~Shader();

	void bind() const;
	void reload();
	void load(std::string vertex, std::string fragment);
	
	void uniform(std::string name, Mat4 mat) const;
	void uniform(std::string name, Vec3 vec3) const;
	void uniform(std::string name, Vec2 vec2) const;
	void uniform(std::string name, GLint i) const;
	void uniform(std::string name, bool b) const;

private:
	GLuint loc(std::string name) const;
	static bool validate(GLuint program);

	std::string v_file, f_file;
	GLuint program = 0;
	GLuint v = 0, f = 0;

	void destroy();
};

/// this is very restrictive; it assumes a set number of gl_rgb8 output
/// textures and a floating point depth render buffer.
class Framebuffer {
public:
	Framebuffer(int outputs, Vec2 dim, int samples = 1);
	Framebuffer(const Framebuffer& src) = delete;
	Framebuffer(Framebuffer&& src);
	~Framebuffer();

	static void bind_screen();

	void resize(Vec2 dim, int samples = 1);
	void bind() const;
	bool is_multisampled() const;

	GLuint get_output(int idx) const; 
	
	void blit_to_screen(int idx, Vec2 dim) const;
	void clear(Vec4 col) const;

private:
	void create();
	void destroy();

	std::vector<GLuint> output_textures;
	GLuint depth_rbo = 0;
	GLuint framebuffer = 0;

	int w = 0, h = 0, s = 0;

	friend class Resolve;
};

class Resolve {
public:
	static void to_screen(const Framebuffer& framebuffer, int buf);

private:
	static void init();
	static void destroy();

	static inline Shader resolve_shader;
	static inline GLuint vao = 0, vbo = 0;
	static inline const GLfloat data[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	friend void setup();
	friend void shutdown();
};

}
