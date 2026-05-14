#include "Shader.h"

Shader::Shader(){ }
Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
	generate(vertexPath, fragmentPath, geometryPath);
}

void Shader::generate(const char* vertexPath, const char* fragmentPath, const char* geometryPath){
	int success;
	char infoLog[1024];

	GLuint vertexShader = compileShader(vertexPath, GL_VERTEX_SHADER);
	GLuint fragmentShader = compileShader(fragmentPath, GL_FRAGMENT_SHADER);
	GLuint geometryShader;
	if (geometryPath) {
		geometryShader = compileShader(geometryPath, GL_GEOMETRY_SHADER);
	}

	id = glCreateProgram();
	glAttachShader(id, vertexShader);
	glAttachShader(id, fragmentShader);
	if (geometryPath)
		glAttachShader(id, geometryShader);
	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(id, 1024, NULL, infoLog);
		std::cout << "SHADER LINKING ERROR " << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	if (geometryPath)
		glDeleteShader(geometryShader);
}

void Shader::activate(){
	glUseProgram(id);
}

std::string Shader::loadShaderSrc(const char* filename){
	std::ifstream file;
	std::stringstream buff;

	std::string ret = "";

	file.open(filename);
	if (file.is_open()) {
		buff << file.rdbuf();
		ret = buff.str();
	}
	else {
		std::cout << "COULD NOT OPEN " << filename << std::endl;
	}
	file.close();
	return ret;
}

GLuint Shader::compileShader(const char* name, GLenum type){
	int success;
	char infoLog[1024];

	GLuint ret = glCreateShader(type);
	std::string shaderSrc = loadShaderSrc(name);
	const GLchar* shader = shaderSrc.c_str();
	glShaderSource(ret, 1, &shader, NULL);
	glCompileShader(ret);

	glGetShaderiv(ret, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(ret, 1024, NULL, infoLog);
		std::cout << "ERROR WITH SHADER COMP " << infoLog << std::endl;
	}
	return ret;
}

void Shader::setUniformBlockBinding(const char* name, int idx){
	glUniformBlockBinding(id, glGetUniformBlockIndex(id, name), idx);
}

void Shader::setBool(const std::string& name, bool value) const{
	glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const{
	glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const{
	glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::set2Float(const std::string& name, float v1, float v2) const{
	glUniform2f(glGetUniformLocation(id, name.c_str()), v1, v2);
}

void Shader::set2Float(const std::string& name, glm::vec2 v) const{
	set2Float(name, v.x, v.y);
}

void Shader::set3Float(const std::string& name, glm::vec3 v) const{
	set3Float(name, v.x, v.y, v.z);
}

void Shader::set3Float(const std::string& name, float v1, float v2, float v3) const{
	glUniform3f(glGetUniformLocation(id, name.c_str()), v1, v2, v3);
}

void Shader::set4Float(const std::string& name, glm::vec4 v) const{
	set4Float(name, v.x, v.y, v.z, v.w);
}

void Shader::set4Float(const std::string& name, float v1, float v2, float v3, float v4) const{
	glUniform4f(glGetUniformLocation(id, name.c_str()), v1, v2, v3, v4);
}

void Shader::setMat4(const std::string& name, glm::mat4 val) const{
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(val));
}

Shader::~Shader(){
	glDeleteProgram(id);
}
