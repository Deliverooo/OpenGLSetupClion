#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "glm/glm.hpp"

class Transform {

    public:

        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;

        Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale){
            this->position = position;
            this->rotation = rotation;
            this->scale = scale;
        }

};
#endif //TRANSFORM_H
