#ifndef TEXTURE_H
#define TEXTURE_H
#include "glad/glad.h"
#include <iostream>
#include "stb_image.h"

class Texturefile {
    private:

        GLuint id;
        int width, height, nrChannels;

    public:

        Texturefile(const std::string &filepath, const std::string &type) {


            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_2D, id);
            // set the texture wrapping parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // set texture filtering parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            stbi_set_flip_vertically_on_load(GL_TRUE);

            unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);

            if (nrChannels == 3) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            } else if (nrChannels == 4) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }

            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);
        }

        GLuint getId() const{
                return id;
        }
};

#endif //TEXTURE_H
