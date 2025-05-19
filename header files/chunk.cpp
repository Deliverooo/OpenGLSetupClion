#include "chunk.h"

Chunk::Chunk() {

}
void Chunk::draw(const Shader &shader) {

    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_HEIGHT; j++) {
            for (int k = 0; k < CHUNK_SIZE; k++) {

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(i, j-1, k));

                shader.uploadUniformMatrix4f("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
    }
}

void Chunk::generateChunk() {

    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_HEIGHT; j++) {
            for (int k = 0; k < CHUNK_SIZE; k++) {

                data[i][j][k] = i, j, k;
            }
        }
    }
}
