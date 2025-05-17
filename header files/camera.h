#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "glm/detail/type_vec.hpp"

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    JUMP,
};

const GLfloat YAW = 90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 5.0f;
const GLfloat SENSITIVITY = 0.08f;
const GLfloat ZOOM = 45.0f;
const GLfloat GRAVITY = 9.8f;
const GLfloat JUMP_HEIGHT = 2.0f;
const GLboolean IN_AIR = GL_FALSE;


class Camera {
    public:
        glm::vec3 cameraPosition;
        glm::vec3 cameraFront;
        glm::vec3 cameraUp;
        glm::vec3 cameraRight;
        glm::vec3 cameraWorldUp;

        float yaw;
        float pitch;
        float movementSpeed;
        float mouseSensitivity;
        float zoom;
        float jumpVelocity;
        bool inAir;

        Camera(glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f),
            float yaw = YAW, float pitch = PITCH, float speed = SPEED, float sensitivity = SENSITIVITY, float zoom = ZOOM, bool inAir = IN_AIR) {

            this->cameraPosition = position;
            this->cameraWorldUp = up;
            this->cameraFront = cameraFront;
            this->yaw = yaw;
            this->pitch = pitch;
            this->movementSpeed = speed;
            this->mouseSensitivity = sensitivity;
            this->zoom = zoom;
            this->inAir = inAir;
            updateCameraVectors();
        }

        glm::mat4 getViewMatrix() {
            return glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
        }

        void processKeyboard(Camera_Movement direction, float deltaTime) {

            float velocity = this->movementSpeed * deltaTime;

            if (direction == FORWARD) {
                cameraPosition += glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z)) * velocity;
            }
            if (direction == BACKWARD) {
                cameraPosition -= glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z)) * velocity;
            }
            if (direction == LEFT) {
                cameraPosition -= cameraRight * velocity;
            }
            if (direction == RIGHT) {
                cameraPosition += cameraRight * velocity;
            }
        }

        void jump(float deltaTime) {

        }

        void update() {

        }

        void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {

            xoffset *= this->mouseSensitivity;
            yoffset *= this->mouseSensitivity;

            yaw += xoffset;
            pitch += yoffset;

            if (constrainPitch) {
                if (pitch > 89.0f) {
                    pitch = 89.0f;
                }
                if (pitch < -89.0f) {
                    pitch = -89.0f;
                }
            }
            updateCameraVectors();
        }
        void processMouseScroll(float yoffset) {
            zoom -= yoffset;
            if (zoom < 1.0f) {
                zoom = 1.0f;
            }
            if (zoom > 45.0f) {
                zoom = 45.0f;
            }
        }

        void updateCameraVectors() {

            //ok, so lets define what is going on here.
            // the camera direction vector (or front) is, well the direction the camera is facing.
            //the camera right vector is calculated by taking the cross product of the camera direction and world up
            //then normalizing the result.
            // if you want to be 𝓯𝓪𝓷𝓬𝔂, you could hypothetically write it like this:

            //d̂ = ⌈cos(yθ) * cos(pθ)⌉
            //    |     sin(pθ)     |
            //    ⌊sin(yθ) * cos(pθ)⌋
            //r̂ = ∥d̂ × û∥
            //û = d̂ × r̂
            //{d̂, r̂, û} | {d̂, r̂, û} ∈ ℝ^3

            glm::vec3 cameraDirection;
            cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            cameraDirection.y = sin(glm::radians(pitch));
            cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

            cameraFront = glm::normalize(cameraDirection);
            cameraRight = glm::normalize(glm::cross(cameraDirection, cameraWorldUp));
            cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
        }
};

#endif //CAMERA_H
