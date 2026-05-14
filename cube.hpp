#ifndef CUBE_HPP
#define CUBE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Shader.h"
#include "Texture.h"
#include "glMemory.hpp"
#include "material.h"


class Cube {
public:
	glm::vec3 pos;
	glm::vec3 size;
    Material material;
    std::vector<Texture> textures;
    static ArrayObject VAO;
    static bool initialized;

    Cube(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f), Material material = Material::emerald)
			: pos(pos), size(size), material(material){	
        if (!initialized) {
            init();
        }
    }

    Cube(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f), std::vector<Texture> textures = { })
            : pos(pos), size(size), textures(textures){
        if (!initialized)
            init();
        }
	static void init() {
        initialized = true;
        int noVertices = 36;
        float vertices[] = {
            // position                 normal              texcoord
            -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,

             0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,    1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,    0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,    1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,    1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,    1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,    0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,    0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,    1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,    1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,    1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,    0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,    0.0f, 1.0f
        };

        std::vector<unsigned int> indicies(noVertices);
        for (unsigned int i = 0; i < noVertices; i++) {
            indicies[i] = i;
        }

        VAO.generate();
        VAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
        VAO["EBO"] = BufferObject(GL_ELEMENT_ARRAY_BUFFER);
        VAO.bind();

        VAO["VBO"].bind();
        VAO["VBO"].setData<float>(sizeof(vertices) / sizeof(vertices[0]), vertices, GL_STATIC_DRAW);
        VAO["EBO"].bind();
        VAO["EBO"].setData<unsigned int>(noVertices, &indicies[0], GL_STATIC_DRAW);
        VAO["VBO"].setAttPointer<float>(0, 3, GL_FLOAT, 8, 0);
        VAO["VBO"].setAttPointer<float>(1, 3, GL_FLOAT, 8, 3);
        VAO["VBO"].setAttPointer<float>(2, 2, GL_FLOAT, 8, 6);

        VAO["VBO"].clear();

        ArrayObject::clear();
	}

    virtual void render(Shader &shader) {
        shader.activate();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::scale(model, size);
        shader.setMat4("model", model);
        shader.set3Float("light.ambient", Material::white_plastic.ambient);
        shader.set3Float("light.diffuse", Material::white_plastic.diffuse);
        shader.set3Float("light.specular", Material::white_plastic.specular);

        // material properties
        shader.setFloat("material.shininess", 64.0f);
        shader.setInt("material.diffuse", 0);
        shader.setInt("material.specular", 1);
        for (int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            textures[i].bind();
        }


        VAO.bind();
        VAO.draw(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    void cleanup() {
        VAO.cleanup();
    }

    virtual ~Cube() = default;
};


#endif // !CUBE_HPP
