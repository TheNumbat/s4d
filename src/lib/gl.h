
#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

#include "math.h"

class GL_Mesh {
public:
	struct Vert {
		Vec3 pos;
		Vec3 norm;
	};

	GL_Mesh();
	GL_Mesh(const std::vector<Vert>& vertices);
	GL_Mesh(const GL_Mesh& src) = delete;
	GL_Mesh(GL_Mesh&& src);
	~GL_Mesh();

	/// Assumes proper shader is already bound
	void render();
	void update(const std::vector<Vert>& vertices);

private:
	void create();
	void destroy();

	GLuint vao = 0, vbo = 0;
	GLuint n_elem = 0;
};

class GL_Lines {
public:
	GL_Lines(float thickness);
	GL_Lines(const GL_Lines& src) = delete;
	GL_Lines(GL_Lines&& src);
	~GL_Lines();

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

class GL_Shader {	

public:
    GL_Shader(std::string vertex_file, std::string fragment_file);
	GL_Shader(const GL_Shader& src) = delete;
	GL_Shader(GL_Shader&& src);
    ~GL_Shader();

	void bind();
	void reload();
	GLuint uniform(std::string name) const;

	static bool validate(GLuint program);

private:
	std::string v_file, f_file;
	GLuint program = 0;
	GLuint v = 0, f = 0;

	void load(std::string vertex, std::string fragment);
    void destroy();
};
