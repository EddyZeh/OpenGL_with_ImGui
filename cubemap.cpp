#include "cubemap.h"

Cubemap::Cubemap() 
	: hasTextures(false) {}

void Cubemap::loadTextures(std::string _dir, 
	std::string right, 
	std::string left, 
	std::string top, 
	std::string bottom, 
	std::string front, 
	std::string back){ 
	
	dir = _dir;
	hasTextures = true;
	faces = { right, left, top, bottom, front, back };


	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	stbi_set_flip_vertically_on_load(false);
	int width, height, nChannels;
	for (unsigned int i = 0; i < 6; i++){
		unsigned char* data = stbi_load((dir + "/" + faces[i]).c_str(), &width, &height, &nChannels, 0);
		GLenum colorMode = GL_RED;
		switch (nChannels) {
		case 3:
			colorMode = GL_RGB;
			break;
		case 4:
			colorMode = GL_RGBA;
			break;
		}
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, colorMode, width, height, 0, colorMode, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
void Cubemap::init() {
	VAO.generate();
	VAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
	VAO.bind();

	VAO["VBO"].bind();
	VAO["VBO"].setData<float>(MeshData::Skybox.vertices.size(), MeshData::Skybox.vertices.data(), GL_STATIC_DRAW);
	VAO["VBO"].setAttPointer<float>(0, 3, GL_FLOAT, 3, 0);

	ArrayObject::clear();
}

void Cubemap::render(Shader& shader, glm::mat4 _view, glm::mat4 projection) {
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	shader.activate();

	// set view and projection matrix
	glm::mat4 view = glm::mat4(glm::mat3(_view));
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);

	//Bind VAO
	VAO.bind();
	if (hasTextures) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	}

	VAO.draw(GL_TRIANGLES, 36);
	ArrayObject::clear();

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}

void Cubemap::cleanup() {
	VAO.cleanup();
}
