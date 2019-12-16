
#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

#include "math.h"

class GL_Mesh {
public:
	GL_Mesh();
	GL_Mesh(const std::vector<Vec3>& vertices, const std::vector<GLuint>& elements);
	GL_Mesh(const GL_Mesh& src) = delete;
	GL_Mesh(GL_Mesh&& src);
	~GL_Mesh();

	/// Assumes proper shader is already bound
	void render();
	void update(const std::vector<Vec3>& vertices, const std::vector<GLuint>& elements);

private:
	void create();
	void destroy();

	GLuint vao = 0, vbo = 0, ebo = 0;
	GLuint n_elem = 0;
};

class GL_Shader {	

public:
    GL_Shader(std::string vertex_file, std::string fragment_file);
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
