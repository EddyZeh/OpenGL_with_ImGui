#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
	// program id
	unsigned int id;

	Shader();
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

	void generate(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

	// activate shader
	void activate();

	// utility function
	std::string loadShaderSrc(const char* filename);
	GLuint compileShader(const char* name, GLenum type);

	void setUniformBlockBinding(const char* name, int idx);

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void set2Float(const std::string& name, float v1, float v2) const;
	void set2Float(const std::string& name, glm::vec2 v) const;
	void set3Float(const std::string& name, glm::vec3 v) const;
	void set3Float(const std::string& name, float v1, float v2, float v3) const;
	void set4Float(const std::string& name, glm::vec4 v) const;
	void set4Float(const std::string& name, float v1, float v2, float v3, float v4) const;
	void setMat4(const std::string& name, glm::mat4 val) const;
	~Shader();

};

#endif // !SHADER_H

