#include "shader.h"
#include "texture.h"
#include "glad/glad.h"

#ifndef MESH_H
#define MESH_H

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};
class Mesh {

    public:
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        std::vector<Texture> textures;

        Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
        void draw(Shader shader);


    private:
        GLuint vaoID, vboID, eboID;

};

#endif