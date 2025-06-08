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

enum View_Mode {
    WALK,
    FLY,
    ORBIT,
};

// default camera attributes
constexpr GLfloat YAW = 90.0f;
constexpr GLfloat PITCH = 0.0f;
constexpr GLfloat SPEED = 5.0f;
constexpr GLfloat JUMP_SPEED = 1.0f;
constexpr GLfloat SENSITIVITY = 0.08f;
constexpr GLfloat ZOOM = 70.0f;
constexpr GLfloat GRAVITY = -20.0f;
constexpr GLfloat JUMP_HEIGHT = 7.0f;
constexpr bool IN_AIR = false;

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
            float yaw = YAW, float pitch = PITCH, float speed = SPEED, float sensitivity = SENSITIVITY, float zoom = ZOOM, bool inAir = IN_AIR, float jumpSpeed = JUMP_SPEED ) {

            this->cameraPosition = position;
            this->cameraWorldUp = up;
            this->cameraFront = cameraFront;
            this->yaw = yaw;
            this->pitch = pitch;
            this->movementSpeed = speed;
            this->jumpVelocity = jumpSpeed;
            this->mouseSensitivity = sensitivity;
            this->zoom = zoom;
            this->inAir = inAir;
            updateCameraVectors();
        }

        // no discard basically means "If you want my return value, you better use it!"
        [[nodiscard]] glm::mat4 getViewMatrix() const {
            return glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
        }

        void processKeyboard(const Camera_Movement direction, const double deltaTime) {

            const float velocity = this->movementSpeed * static_cast<float>(deltaTime);

            if (direction == FORWARD) {
                cameraPosition += glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z)) * velocity;
            }
            if (direction == BACKWARD) {
                cameraPosition -= glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z)) * velocity;
            }
            if (direction == LEFT) {
                cameraPosition -= glm::normalize(cameraRight) * velocity;
            }
            if (direction == RIGHT) {
                cameraPosition += glm::normalize(cameraRight) * velocity;
            }
            if (direction == JUMP) {
                jump();
            }
        }

        void jump() {
            if (!inAir) {
                jumpVelocity = JUMP_SPEED * JUMP_HEIGHT;
                inAir = GL_TRUE;
            }
        }

        void applyGravity(const float deltaTime) {

            jumpVelocity += GRAVITY * deltaTime;
            cameraPosition += glm::vec3(0.0f, jumpVelocity * deltaTime, 0.0f);

            if (cameraPosition.y <= 1) {
                jumpVelocity = 0.0f;
                cameraPosition.y = 1.0f;
                inAir = GL_FALSE;
            }
        }

        void update(const float deltaTime) {
            applyGravity(deltaTime);
        }

        void processMouseMovement(double xoffset, double yoffset, const GLboolean constrainPitch = true) {

            xoffset *= this->mouseSensitivity;
            yoffset *= this->mouseSensitivity;

            yaw += static_cast<float>(xoffset);
            pitch += static_cast<float>(yoffset);

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

        void processMouseScroll(const double yoffset) {

            zoom -= static_cast<float>(yoffset);
            if (zoom < 1.0f) {
                zoom = 1.0f;
            }
            if (zoom > ZOOM) {
                zoom = ZOOM;
            }
        }

        void updateCameraVectors() {

            //ok, so let's define what is going on here.
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
