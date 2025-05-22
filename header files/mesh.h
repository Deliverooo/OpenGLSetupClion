#include "shader.h"
#include "glad/glad.h"

#ifndef MESH_H
#define MESH_H

struct Texture {
    GLuint id;
    std::string type;
    std::string path;
};

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

        Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures) {
            this->vertices = vertices;
            this->indices = indices;
            this->textures = textures;

            prepareMesh();

        }
        void draw(Shader shader) {

            GLuint diffuseNr = 1;
            GLuint specularNr = 1;

            for (GLuint i = 0; i < textures.size(); i++) {

                glActiveTexture(GL_TEXTURE0 + i);

                std::string name = textures[i].type;
                std::string num;
                if (name == "diffuseTex") {
                    num = std::to_string(diffuseNr++);
                } else if (name == "specularTex") {
                    num = std::to_string(specularNr++);
                }
                shader.uploadInt(("material." + name + num).c_str(), i);
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
            }
        }
    private:
        GLuint vaoID, vboID, eboID;

        void prepareMesh() {
            //generates the vertex arrays and buffers
            glGenVertexArrays(1, &vaoID);
            glGenBuffers(1, &vboID);
            glGenBuffers(1, &eboID);

            //binds the vertex array, so it is the one that is currently active
            glBindVertexArray(vaoID);

            //binds the buffer so we can store data in it
            glBindBuffer(GL_ARRAY_BUFFER, vaoID);
            glBufferData(GL_ARRAY_BUFFER, vertices.size(), &vertices[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size(), &indices[0], GL_STATIC_DRAW);
            //let me break it down for you Mark.
            //the size is the number of elements in the attribute. e.g. a vec3 would have 3 and a vec2 would have 2
            //the stride is the total number of elements multiplied by the float size in bytes. e.g. a vec2 and vec3 would have a combined size of 5
            // the pointer is the offset for each attribute from the previous one
            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            glEnableVertexAttribArray(0);
            //texture coordinates attribute
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
            glEnableVertexAttribArray(1);
            //normals attribute
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
            glEnableVertexAttribArray(2);

            // You can unbind the VAO afterward so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
            // VAOs requires a call to glBindVertexArray anyway so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
            glBindVertexArray(0);
        }

};

#endif