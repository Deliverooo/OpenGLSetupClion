#ifndef VAO_H
#define VAO_H
#include "glad/glad.h"

class Vao {
    private:
        GLuint vaoID;

    public:
        void createVao(float vertices[]) {

            //generates the vertex arrays
            glGenVertexArrays(1, &vaoID);

            //binds the vertex array, so it is the one that is currently active
            glBindVertexArray(vaoID);

            //let me break it down for you Mark.
            //the size is the number of elements in the attribute. e.g. a vec3 would have 3 and a vec2 would have 2
            //the stride is the total number of elements multiplied by the float size in bytes. e.g. a vec2 and vec3 would have a combined size of 5
            // the pointer is the offset for each attribute from the previous one
            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            //texture coordinates attribute
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            //normals attribute
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
            glEnableVertexAttribArray(2);

            // You can unbind the VAO afterward so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
            // VAOs requires a call to glBindVertexArray anyway so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        }
        GLuint getVaoID() {
            return vaoID;
        }
};
#endif //VAO_H
