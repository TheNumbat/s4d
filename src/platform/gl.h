
#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

#include "../lib/math.h"

namespace GL {

void setup();
void shutdown();
void flush_if_nvidia();
std::string version();
std::string renderer();

void global_params();
void clear_screen(Vec4 col);
void viewport(Vec2 dim);

enum class Opt {
	wireframe,
	offset,
	culling
};

void enable(Opt opt);
void disable(Opt opt);

void color_mask(bool enable);

class Mesh {
public:
	typedef GLuint Index;
	struct Vert {
		Vec3 pos;
		Vec3 norm;
		GLuint id;
	};

	Mesh();
	Mesh(std::vector<Vert>&& vertices, std::vector<Index>&& indices);
	Mesh(const Mesh& src) = delete;
	Mesh(Mesh&& src);
	~Mesh();

	void operator=(const Mesh& src) = delete;
	void operator=(Mesh&& src);

	/// Assumes proper shader is already bound
	void render() const;
	void update(std::vector<Vert>&& vertices, std::vector<Index>&& indices);

	BBox bbox() const;
	const std::vector<Vert>& verts() const;
	const std::vector<Index>& indices() const;

private:
	void create();
	void destroy();

	BBox _bbox;
	GLuint vao = 0, vbo = 0, ebo = 0;
	GLuint n_elem = 0;

	std::vector<Vert> _verts;
	std::vector<Index> _idxs;

	friend class Instances;
};

class Instances {
public:
	Instances(GL::Mesh&& mesh);
	Instances(const Instances& src) = delete;
	Instances(Instances&& src);
	~Instances();

	void operator=(const Instances& src) = delete;
	void operator=(Instances&& src);

	void render();
	void add(Mat4 transform);
	void clear();

private:
	void create();
	void destroy();
	void update();

	GLuint vbo = 0;
	bool dirty = false;

	Mesh mesh;
	std::vector<Mat4> transforms;
};

class Lines {
public:
	Lines(float thickness);
	Lines(const Lines& src) = delete;
	Lines(Lines&& src);
	~Lines();

	void operator=(const Lines& src) = delete;
	void operator=(Lines&& src);

	/// Assumes proper shader is already bound
	void render(bool smooth) const;
	void add(Vec3 start, Vec3 end, Vec3 color);
	void pop();
	void clear();

private:
	void create();
	void destroy();
	void update() const;

	mutable bool dirty = false;
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

	void operator=(const Shader& src) = delete;
	void operator=(Shader&& src);

	void bind() const;
	void load(std::string vertex, std::string fragment);
	
	void uniform(std::string name, Mat4 mat) const;
	void uniform(std::string name, Vec3 vec3) const;
	void uniform(std::string name, Vec2 vec2) const;
	void uniform(std::string name, GLint i) const;
	void uniform(std::string name, GLuint i) const;
	void uniform(std::string name, GLfloat f) const;
	void uniform(std::string name, bool b) const;
	void uniform(std::string name, int count, const Vec2 items[]) const;

private:
	GLuint loc(std::string name) const;
	static bool validate(GLuint program);

	GLuint program = 0;
	GLuint v = 0, f = 0;

	void destroy();
};

/// this is very restrictive; it assumes a set number of gl_rgb8 output
/// textures and a floating point depth render buffer.
class Framebuffer {
public:
	Framebuffer();
	Framebuffer(int outputs, Vec2 dim, int samples = 1, bool depth = true);
	Framebuffer(const Framebuffer& src) = delete;
	Framebuffer(Framebuffer&& src);
	~Framebuffer();

	void operator=(const Framebuffer& src) = delete;
	void operator=(Framebuffer&& src);

	static void bind_screen();

	void setup(int outputs, Vec2 dim, int samples, bool d);
	void resize(Vec2 dim, int samples = 1);
	void bind() const;
	bool is_multisampled() const;

	GLuint get_output(int buf) const; 
	GLuint get_depth() const; 

	bool can_read_at() const;
	void read_at(int buf, int x, int y, GLubyte* data) const;
	void read(int buf, GLubyte* data) const;
	
	void blit_to_screen(int buf, Vec2 dim) const;
	void blit_to(int buf, const Framebuffer& fb, bool avg = true) const;

	void clear(int buf, Vec4 col) const;
	void clear_d() const;

private:
	void create();
	void destroy();

	std::vector<GLuint> output_textures;
	GLuint depth_tex = 0;
	GLuint framebuffer = 0;

	int w = 0, h = 0, s = 0;
	bool depth = true;

	friend class Effects;
};

class Effects {
public:
	static void resolve_to_screen(int buf, const Framebuffer& framebuffer);
	static void resolve_to(int buf, const Framebuffer& from, const Framebuffer& to, bool avg = true);

	static void outline(const Framebuffer& from, const Framebuffer& to, Vec3 color, Vec2 min, Vec2 max);

private:
	static void init();
	static void destroy();

	static inline Shader resolve_shader, outline_shader, outline_shader_ms;
	static inline GLuint vao;
	static inline const Vec2 screen_quad[] = {
		{-1.0f,  1.0f},
		{-1.0f, -1.0f},
		{1.0f,  1.0f},
		{1.0f, -1.0f}
	};

	friend void setup();
	friend void shutdown();

	static const std::string effects_v;
	static const std::string outline_f, outline_ms_f_33, outline_ms_f_4;
	static const std::string resolve_f;
};

namespace Shaders {
	extern const std::string line_v, line_f;
	extern const std::string mesh_v, mesh_f;
	extern const std::string inst_v;
}

}
