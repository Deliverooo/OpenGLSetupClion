#include <iomanip>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "header files/shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <iostream>

#include "header files/camera.h"
#include "header files/chunk.h"
#include "header files/model.h"
#include "header files/texture.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void OrbitLight(glm::mat4 &light, glm::vec3 &lightPos, const glm::vec3 &pivotPos, float radius, const int offsetMultiplier);
void uploadPointLightUniforms(Shader &shader, int index, glm::vec3 &lightPos, glm::vec3 ambient, glm::vec3 lightColour,
    float constant, float linear, float quadratic);
void createSpotLight(Shader &shader, const int index, glm::vec3 &lightPos, glm::vec3 &lightDirection, glm::vec3 ambient, glm::vec3 diffuse,
    float cutOffAngle, float outerCutOffAngle, float constant, float linear, float quadratic);

#define constexpr GLuint WIDTH = 1920;
constexpr GLuint HEIGHT = 1080;

typedef glm::vec3 Vector3f;
typedef glm::mat4 Matrix4f;

double deltaTime = 0.0f;
double lastFrame = 0.0f;
double currentFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 1.0f, 0.0f));

float lastScroll = 0;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

Vector3f lightPos = glm::vec3(2.0f, 1.0f, 2.0f);
Vector3f lightPos2 = glm::vec3(3.0f, 1.0f, 3.0f);

glm::vec3 spotLightPos = glm::vec3(5, 1, 5);
glm::vec3 spotLightPos2 = glm::vec3(5, 1, 5);
glm::vec3 spotLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);

Vector3f spotLightCol = Vector3f(0.5f);
Vector3f spotLightCol2 = Vector3f(0.5f);

glm::vec3 lightDir = glm::vec3(0.0f, -0.7f, 0.5f);


int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "All hail Orbo", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //enables v-sync
    glfwSwapInterval(1);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    Shader shader("resources/shaders/vertex.glsl", "resources/shaders/fragment.glsl"); // you can name your shader files however you like
    Shader lightShader("resources/shaders/light_vertex.glsl", "resources/shaders/light_fragment.glsl");
    //makes sure that the shader is currently being used
    shader.use();

    Chunk chunk;

    Model orboModel("resources/models/orbo/Orbo_Obj.obj");
    Model floorTiles("resources/models/floor Tiles/Floor.obj");

    glEnable(GL_DEPTH_TEST);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();
        processInput(window);
        camera.update(static_cast<float>(deltaTime));
        glClearColor(0.0f, 0.11f, 0.21f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(camera.cameraPosition, camera.cameraPosition + camera.cameraFront, camera.cameraUp);
        shader.uploadUniformMatrix4f("projection", projection);
        shader.uploadUniformMatrix4f("view", view);
        shader.uploadUniformVector3f("viewPos", camera.cameraFront);
        shader.uploadInt("material.specularRoughness", 4);
        shader.setFloat("material.specularIntensity", 0.1f);
        shader.uploadUniformVector3f("material.specularTint", glm::vec3(1.0f, 1.0f, 1.0f));

        uploadPointLightUniforms(shader, 0, lightPos, glm::vec3(0.1f), glm::vec3(1.0f), 0.7f, 0.09f, 0.032f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        shader.uploadUniformMatrix4f("model", model);
        orboModel.draw(shader);

        glm::mat4 floor = glm::mat4(1.0f);
        floor = glm::translate(floor, glm::vec3(0.0f, -1.0f, 1.0f));
        floor = glm::rotate(floor, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        floor = glm::scale(floor, glm::vec3(1.0f));
        shader.uploadUniformMatrix4f("model", floor);
        floorTiles.draw(shader);

        glm::mat4 light = glm::mat4(1.0f);
        OrbitLight(light, lightPos, glm::vec3(0.0f, 0.0f, 0.0f), 3.0f, 1);


        glfwSwapBuffers(window);
        glfwPollEvents();

        lastFrame = glfwGetTime();
        deltaTime = lastFrame - currentFrame;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.processKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.processKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.processKeyboard(RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.processKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.processKeyboard(JUMP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        std::cout << "printing" << std::endl;
    }
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    std::cout << "Frame buffer size: " << width << ", " << height << std::endl;
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float mouseDx = (float)xpos - lastX;
    float mouseDy = (float)lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(mouseDx, mouseDy);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll(yoffset);
}
void OrbitLight(glm::mat4 &light, glm::vec3 &lightPos, const glm::vec3 &pivotPos, const float radius, const int offsetMultiplier) {

    float dX = pivotPos.x + offsetMultiplier * ((sin(glfwGetTime()) * radius));
    float dY = pivotPos.y + offsetMultiplier * (sin(glfwGetTime()) * radius / 8.0f);
    float dZ = pivotPos.z + offsetMultiplier * (cos(glfwGetTime()) * radius);

    lightPos.x = dX;

    lightPos.z = dZ;

    light = glm::translate(light, lightPos);
    light = glm::scale(light, glm::vec3(0.35f));
}
void createSpotLight(Shader &shader, const int index, glm::vec3 &lightPos, glm::vec3 &lightDirection, glm::vec3 ambient, glm::vec3 lightColour,
    float cutOffAngle, float outerCutOffAngle, float constant, float linear, float quadratic) {

    shader.uploadUniformVector3f("spotlights[" + std::to_string(index) + "].position", lightPos);
    shader.uploadUniformVector3f("spotlights[" + std::to_string(index) + "].direction", lightDirection);
    shader.uploadUniformVector3f("spotlights[" + std::to_string(index) + "].ambient", ambient);
    shader.uploadUniformVector3f("spotlights[" + std::to_string(index) + "].lightColour", lightColour);
    shader.setFloat("spotlights[" + std::to_string(index) + "].cutOffAngle", cutOffAngle);
    shader.setFloat("spotlights[" + std::to_string(index) + "].outerCutOffAngle", outerCutOffAngle);
    shader.setFloat("spotlights[" + std::to_string(index) + "].constant", constant);
    shader.setFloat("spotlights[" + std::to_string(index) + "].linear", linear);
    shader.setFloat("spotlights[" + std::to_string(index) + "].quadratic", quadratic);
}
void uploadPointLightUniforms(Shader &shader, const int index, Vector3f &lightPos, Vector3f ambient, Vector3f lightColour,
    float constant, float linear, float quadratic) {

    shader.uploadUniformVector3f("pointLights[" + std::to_string(index) + "].lightPos", lightPos);
    shader.uploadUniformVector3f("pointLights[" + std::to_string(index) + "].ambient", ambient);
    shader.uploadUniformVector3f("pointLights[" + std::to_string(index) + "].lightColour", lightColour);
    shader.setFloat("pointLights[" + std::to_string(index) + "].constant", constant);
    shader.setFloat("pointLights[" + std::to_string(index) + "].linear", linear);
    shader.setFloat("pointLights[" + std::to_string(index) + "].quadratic", quadratic);
}


