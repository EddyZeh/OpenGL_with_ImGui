#include "mesh.h"

std::vector<Vertex> Vertex::genList(const std::vector<float> vertices) {
	int stride = sizeof(Vertex) / sizeof(float);

	int noVertices =vertices.size() / stride;
	std::vector<Vertex> ret(noVertices);

	for (unsigned int i = 0; i < noVertices; i++) {
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
		: vertices(vertices), indices(indices), textures(textures), noTex(false){
	setupMesh();
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, aiColor4D diff, aiColor4D spec)
		: vertices(vertices), indices(indices), diff(diff), spec(spec), noTex(true){
	setupMesh();
}

void Mesh::render(Shader& shader) {
	if (noTex) {
		shader.set3Float("material.diffuseColor", diff.r, diff.g, diff.b);
		shader.set3Float("material.specularColor", spec.r, spec.g, spec.b);
		shader.setBool("noTex", noTex);
	}
	else {
		shader.setBool("noTex", noTex);
		unsigned int diffuseN = 0;
		unsigned int specularN = 0;

		for (unsigned int i = 0; i < textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);

			std::string name;
			switch (textures[i].type) {
			case aiTextureType_DIFFUSE:
				name = "diffuseColor";
				break;
			case aiTextureType_SPECULAR:
				name = "specularColor";
				break;

			default:
				continue;
			}

			shader.setInt("material." + name, i);
			textures[i].bind();
		}
	}
	VAO.bind();
	VAO.draw(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
	ArrayObject::clear();
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::cleanup() {
	VAO.cleanup();
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
