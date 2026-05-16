#include "mesh.h"

std::vector<Vertex> Vertex::genList(float* vertices, int noVertices) {
	std::vector<Vertex> ret(noVertices);

	int stride = sizeof(Vertex) / sizeof(float);

	for (int i = 0; i < noVertices; i++) {
		ret[i].Position = glm::vec3(
			vertices[i * stride + 0],
			vertices[i * stride + 1],
			vertices[i * stride + 2]
		);

		ret[i].Normal = glm::vec3(
			vertices[i * stride + 3],
			vertices[i * stride + 4],
			vertices[i * stride + 5]
		);

		ret[i].TexCoord = glm::vec2(
			vertices[i * stride + 6],
			vertices[i * stride + 7]
		);
	}
	return ret;
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
		: vertices(vertices), indices(indices), textures(textures){
	setupMesh();
}

void Mesh::render(Shader& shader) {
	unsigned int diffuseN = 0;
	unsigned int specularN = 0;

	for (unsigned int i = 0; i < textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);

		std::string name;
		switch (textures[i].type) {
		case aiTextureType_DIFFUSE:
			name = "diffuse" + std::to_string(i);
			break;
		case aiTextureType_SPECULAR:
			name = "specular" + std::to_string(i);
			break;

		default:
			continue;
		}

		shader.setInt("material." + name, i);
		textures[i].bind();
	}

	glActiveTexture(GL_TEXTURE0);

	VAO.bind();
	VAO.draw(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
}

void Mesh::setupMesh() {
	VAO.generate();

	VAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
	VAO["EBO"] = BufferObject(GL_ELEMENT_ARRAY_BUFFER);

	VAO.bind();

	VAO["VBO"].bind();
	VAO["VBO"].setData<Vertex>(vertices.size(), &vertices[0], GL_STATIC_DRAW);
	
	VAO["EBO"].bind();
	VAO["EBO"].setData<GLuint>(static_cast<GLuint>(indices.size()), &indices[0], GL_STATIC_DRAW);

	VAO["VBO"].setAttPointer<GLfloat>(0, 3, GL_FLOAT, 8, 0);
	VAO["VBO"].setAttPointer<GLfloat>(1, 3, GL_FLOAT, 8, 3);
	VAO["VBO"].setAttPointer<GLfloat>(2, 2, GL_FLOAT, 8, 6);

	ArrayObject::clear();
}
