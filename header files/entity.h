#ifndef ENTITY_H
#define ENTITY_H
#include "model.h"
#include "transform.h"
#include "glm/gtc/matrix_transform.hpp"

class Entity {

    public:
        Transform transform;
        Model model;


        Entity(Transform &transform, Model model): transform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f)), model("") {
            this->transform = transform;
            this->model = model;
        }
        void draw(Shader &shader) {

            shader.use();
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, this->transform.position);
            model = glm::rotate(model, this->transform.rotation.x, this->transform.rotation);
            model = glm::rotate(model, this->transform.rotation.y, this->transform.rotation);
            model = glm::rotate(model, this->transform.rotation.z, this->transform.rotation);

            model = glm::scale(model, this->transform.scale);
            shader.uploadUniformMatrix4f("model", model);
            this->model.draw(shader);
        }

};
#endif //ENTITY_H
