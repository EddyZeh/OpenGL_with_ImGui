#ifndef CUBE_HPP
#define CUBE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Shader.h"
#include "Texture.h"
#include "material.h"

#include "modelarray.hpp"

struct CubeInstance {
    glm::vec3 positions;
    glm::vec3 sizes;
};

class Cube : public Model {
public:
    Material material;
    std::vector<Texture> textures;

    Cube(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f), Material material = Material::emerald)
			: Model(pos,size), material(material) {    }

    Cube(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f), std::vector<Texture> textures = { })
            : Model(pos,size), textures(textures) {       }
    void init() {
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
        
        
        meshes.push_back(Mesh(Vertex::genList(vertices, noVertices), indicies));
	}

    virtual void render(Shader &shader) override{
        shader.activate();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::scale(model, size);
        shader.setMat4("model", model);
        shader.set3Float("pointLight.ambient", Material::white_plastic.ambient);
        shader.set3Float("pointLight.diffuse", Material::white_plastic.diffuse);
        shader.set3Float("pointLight.specular", Material::white_plastic.specular);

        // material properties
        shader.setFloat("material.shininess", 64.0f);
        shader.setInt("material.diffuse", 0);
        shader.setInt("material.specular", 1);
        for (int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            textures[i].bind();
        }


        //meshes[0].VAO.bind();
        //meshes[0].VAO.draw(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    void cleanup() {
        meshes[0].VAO.cleanup();
    }

    virtual ~Cube() = default;
};

class CubeArray : public ModelArray<Cube> {
public:

    std::vector<CubeInstance> cubeInstances;

    void init(std::vector<Texture> textures) {
        model = new Cube(glm::vec3(0.0f), glm::vec3(1.0f), textures);
        ModelArray::init();
    }

    void render(Shader& shader) {
        positions.clear();
        sizes.clear();

        for (const auto &CI : cubeInstances) {
            positions.push_back(CI.positions);
            sizes.push_back(CI.sizes);
        }

        ModelArray::render(shader, false);
    }
};


#endif // !CUBE_HPP
