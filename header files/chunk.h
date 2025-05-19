#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "shader.h"

#ifndef CHUNK_H
#define CHUNK_H

class Chunk {


    public:
        static const int CHUNK_SIZE = 16;
        static const int CHUNK_HEIGHT = 1;

        Chunk();

        void draw(const Shader &shader);
        void generateChunk();

    private:
        bool data[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
        glm::vec3 cubes[];
};

#endif //CHUNK_H
